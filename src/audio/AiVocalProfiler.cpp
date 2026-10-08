#include "AiVocalProfiler.h"
#include <algorithm>
#include <numeric>

AiVocalProfiler::AiVocalProfiler()
{
    recordBuffer.reserve(44100 * 6);
}

void AiVocalProfiler::prepare(double sampleRate, int /*samplesPerBlock*/)
{
    currentSampleRate = (sampleRate > 0.0) ? sampleRate : 44100.0;
    maxRecordSamples = static_cast<int>(currentSampleRate * 5.0); // 5 seconds
    reset();
}

void AiVocalProfiler::reset()
{
    isRecording.store(false, std::memory_order_release);
    recordingProgress.store(0.0f, std::memory_order_release);
    totalRecordedSamples = 0;
    recordBuffer.clear();
}

void AiVocalProfiler::startProfiling()
{
    reset();
    recordBuffer.resize(static_cast<size_t>(maxRecordSamples), 0.0f);
    totalRecordedSamples = 0;
    isRecording.store(true, std::memory_order_release);
}

void AiVocalProfiler::cancelProfiling()
{
    reset();
}

void AiVocalProfiler::processBlock(const juce::AudioBuffer<float>& buffer)
{
    if (!isRecording.load(std::memory_order_relaxed))
        return;

    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();
    if (numSamples <= 0 || numChannels <= 0)
        return;

    const float* ch0 = buffer.getReadPointer(0);
    const float* ch1 = (numChannels > 1) ? buffer.getReadPointer(1) : ch0;

    for (int i = 0; i < numSamples; ++i)
    {
        if (totalRecordedSamples < maxRecordSamples)
        {
            float monoSample = 0.5f * (ch0[i] + ch1[i]);
            recordBuffer[static_cast<size_t>(totalRecordedSamples)] = monoSample;
            totalRecordedSamples++;
        }
        else
        {
            break;
        }
    }

    float prog = static_cast<float>(totalRecordedSamples) / static_cast<float>(maxRecordSamples);
    recordingProgress.store(std::clamp(prog, 0.0f, 1.0f), std::memory_order_relaxed);

    if (totalRecordedSamples >= maxRecordSamples)
    {
        isRecording.store(false, std::memory_order_release);
        analyzeRecordedAudio();
    }
}

void AiVocalProfiler::analyzeRecordedAudio()
{
    lastResult = ProfileResult{};

    if (recordBuffer.empty() || totalRecordedSamples < 1000)
    {
        lastResult.isValid = false;
        lastResult.diagnosticSummary = juce::String::fromUTF8(u8"Chưa nhận được tín hiệu âm thanh mẫu.");
        return;
    }

    // 1. Calculate Overall RMS Energy
    double sumSq = 0.0;
    for (int i = 0; i < totalRecordedSamples; ++i)
    {
        sumSq += recordBuffer[static_cast<size_t>(i)] * recordBuffer[static_cast<size_t>(i)];
    }
    float rms = static_cast<float>(std::sqrt(sumSq / totalRecordedSamples));
    float rmsDb = 20.0f * std::log10(std::max(0.00001f, rms));

    if (rmsDb < -48.0f)
    {
        lastResult.isValid = false;
        lastResult.diagnosticSummary = juce::String::fromUTF8(u8"Tín hiệu Micro quá nhỏ (") + juce::String(rmsDb, 1) + juce::String::fromUTF8(u8" dB). Vui lòng tăng Gain Micro hoặc hát gần Micro hơn rồi thử lại!");
        return;
    }

    // 2. Fundamental Frequency (Pitch / F0) Estimation via Autocorrelation
    int minLag = static_cast<int>(currentSampleRate / 450.0); // 450 Hz upper voice fundamental
    int maxLag = static_cast<int>(currentSampleRate / 70.0);  // 70 Hz lower voice fundamental
    
    float bestAutocorr = 0.0f;
    int bestLag = minLag;

    // Analyze middle 2 seconds for stable pitch
    int startIdx = static_cast<int>(totalRecordedSamples * 0.3);
    int windowLen = static_cast<int>(currentSampleRate * 0.8); // 800ms analysis window
    if (startIdx + windowLen + maxLag < totalRecordedSamples)
    {
        for (int lag = minLag; lag <= maxLag; ++lag)
        {
            double corr = 0.0;
            for (int i = 0; i < windowLen; i += 2)
            {
                corr += recordBuffer[static_cast<size_t>(startIdx + i)] * recordBuffer[static_cast<size_t>(startIdx + i + lag)];
            }
            if (corr > bestAutocorr)
            {
                bestAutocorr = static_cast<float>(corr);
                bestLag = lag;
            }
        }
    }

    float estimatedF0 = (bestLag > 0) ? static_cast<float>(currentSampleRate / bestLag) : 150.0f;
    lastResult.fundamentalHz = estimatedF0;

    // Classify Vocal Range
    if (estimatedF0 < 135.0f)
    {
        lastResult.vocalType = VocalType::MaleBaritone;
        lastResult.vocalTypeName = juce::String::fromUTF8(u8"Nam Trầm (Baritone/Bass)");
    }
    else if (estimatedF0 < 200.0f)
    {
        lastResult.vocalType = VocalType::MaleTenor;
        lastResult.vocalTypeName = juce::String::fromUTF8(u8"Nam Cao (Tenor)");
    }
    else if (estimatedF0 < 275.0f)
    {
        lastResult.vocalType = VocalType::FemaleAlto;
        lastResult.vocalTypeName = juce::String::fromUTF8(u8"Nữ Trung / Ấm (Alto)");
    }
    else
    {
        lastResult.vocalType = VocalType::FemaleSoprano;
        lastResult.vocalTypeName = juce::String::fromUTF8(u8"Nữ Cao (Soprano)");
    }

    // 3. Multi-Band Spectral Energy Distribution Analysis (FFT 2048)
    constexpr int FFT_ORDER = 11;
    constexpr int FFT_SIZE = 1 << FFT_ORDER; // 2048
    juce::dsp::FFT fft{ FFT_ORDER };
    juce::dsp::WindowingFunction<float> window{ FFT_SIZE, juce::dsp::WindowingFunction<float>::hann };

    std::array<float, 6> bandEnergy{}; // 0:Sub, 1:Mud, 2:Core, 3:Presence, 4:Sibilance, 5:Air
    int numFrames = 0;

    for (int offset = 0; offset + FFT_SIZE < totalRecordedSamples; offset += FFT_SIZE / 2)
    {
        std::array<float, FFT_SIZE * 2> fftData{};
        std::memcpy(fftData.data(), recordBuffer.data() + offset, FFT_SIZE * sizeof(float));

        window.multiplyWithWindowingTable(fftData.data(), FFT_SIZE);
        fft.performRealOnlyForwardTransform(fftData.data());

        const float binHz = static_cast<float>(currentSampleRate / FFT_SIZE);

        for (int bin = 1; bin < FFT_SIZE / 2; ++bin)
        {
            float freq = bin * binHz;
            float re = fftData[static_cast<size_t>(bin * 2)];
            float im = fftData[static_cast<size_t>(bin * 2 + 1)];
            float p = re * re + im * im;

            if (freq < 120.0f)
                bandEnergy[0] += p; // Sub/Rumble
            else if (freq < 450.0f)
                bandEnergy[1] += p; // Mud (200-400Hz)
            else if (freq < 2000.0f)
                bandEnergy[2] += p; // Core vocal
            else if (freq < 4500.0f)
                bandEnergy[3] += p; // Clarity / Presence
            else if (freq < 8500.0f)
                bandEnergy[4] += p; // Sibilance
            else if (freq < 18000.0f)
                bandEnergy[5] += p; // Air
        }
        numFrames++;
    }

    if (numFrames > 0)
    {
        for (auto& e : bandEnergy) e /= static_cast<float>(numFrames);
    }

    float coreE = std::max(0.00001f, bandEnergy[2]);
    float presenceE = std::max(0.00001f, bandEnergy[3]);

    lastResult.mudEnergyRatio = bandEnergy[1] / coreE;
    lastResult.sibilanceRatio = bandEnergy[4] / presenceE;
    lastResult.airRatio = bandEnergy[5] / presenceE;

    // 4. Generate Diagnostic Summary
    juce::String mudStatus = (lastResult.mudEnergyRatio > 1.3f) ? juce::String::fromUTF8(u8"⚠️ Phòng hơi ù rền (250Hz cao)") : juce::String::fromUTF8(u8" Độ ấm vừa vặn");
    juce::String sibStatus = (lastResult.sibilanceRatio > 1.25f) ? juce::String::fromUTF8(u8"⚠️ Hơi chói âm gió (6kHz cao)") : juce::String::fromUTF8(u8" Âm gió êm ái");
    juce::String airStatus = (lastResult.airRatio < 0.35f) ? juce::String::fromUTF8(u8"⚠️ Micro hơi tối (Thiếu Air 10k)") : juce::String::fromUTF8(u8" Độ sáng tự nhiên");

    lastResult.diagnosticSummary = juce::String::fromUTF8(u8"• Chất giọng: ") + lastResult.vocalTypeName + "\n"
        + juce::String::fromUTF8(u8"• Tần số đục phòng: ") + mudStatus + "\n"
        + juce::String::fromUTF8(u8"• Độ chói sibilance: ") + sibStatus + "\n"
        + juce::String::fromUTF8(u8"• Độ thoát âm (Air): ") + airStatus;

    // 5. Generate Compensation EQ curves for 4 Styles
    // Style 0: Studio Master
    float baseLowCut = (lastResult.mudEnergyRatio > 1.2f) ? -2.5f : ((lastResult.mudEnergyRatio < 0.7f) ? 1.0f : 0.0f);
    float baseHighAir = (lastResult.airRatio < 0.45f) ? 3.0f : 1.5f;
    if (lastResult.sibilanceRatio > 1.3f) baseHighAir = std::min(baseHighAir, 1.5f);

    lastResult.styles[0] = { baseLowCut, 1.5f, baseHighAir }; // Studio Master
    lastResult.styles[1] = { baseLowCut + 2.0f, 2.0f, std::max(0.5f, baseHighAir - 1.0f) }; // Sweet Bolero
    lastResult.styles[2] = { baseLowCut - 1.5f, 2.5f, baseHighAir + 2.0f }; // Remix Pop
    lastResult.styles[3] = { -3.5f, 3.0f, 2.0f }; // Podcast Streamer

    lastResult.isValid = true;
}

AiVocalProfiler::ProfileResult::EqGains AiVocalProfiler::getGainsForStyle(ProfileStyle style) const
{
    int idx = static_cast<int>(style);
    if (idx >= 0 && idx < 4 && lastResult.isValid)
    {
        return lastResult.styles[idx];
    }
    return { 0.0f, 0.0f, 0.0f };
}
