#include "AiNoiseSuppressor.h"
#include <algorithm>

AiNoiseSuppressor::AiNoiseSuppressor()
{
    initializeBandBoundaries();
    channels.resize(2); // Stereo default
}

void AiNoiseSuppressor::initializeBandBoundaries()
{
    // 24 Critical Bark scale frequency bands for 512-point FFT (257 positive bins @ 44.1kHz / 48kHz)
    // 0 Hz to 22,050 Hz distributed logarithmically across human auditory critical bands
    const int boundaries[NUM_BANDS + 1] = {
        0,   1,   2,   3,   5,   7,   10,  14, 
        19,  25,  32,  41,  52,  65,  81,  100,
        122, 148, 178, 208, 228, 240, 248, 253, 256
    };

    for (int i = 0; i <= NUM_BANDS; ++i)
    {
        bandBoundaries[i] = boundaries[i];
    }
}

void AiNoiseSuppressor::prepare(double sampleRate, int /*samplesPerBlock*/)
{
    currentSampleRate = (sampleRate > 0.0) ? sampleRate : 44100.0;
    channels.resize(2);
    reset();
}

void AiNoiseSuppressor::reset()
{
    for (auto& ch : channels)
    {
        ch.inputFifo.fill(0.0f);
        ch.outputAccum.fill(0.0f);
        ch.fifoIndex = 0;

        ch.noisePsd.fill(0.0001f);
        ch.speechPsd.fill(0.001f);
        ch.lateReverbPsd.fill(0.00001f);
        ch.smoothGains.fill(1.0f);

        ch.vadEnergyTracker = 0.001f;
        ch.vadNoiseFloor = 0.0001f;
        ch.speechProbability = 0.0f;
    }

    currentNoiseReductionDb.store(0.0f, std::memory_order_relaxed);
    currentVoiceProbability.store(0.0f, std::memory_order_relaxed);
}

void AiNoiseSuppressor::process(juce::AudioBuffer<float>& buffer)
{
    if (!enabled.load(std::memory_order_relaxed))
    {
        currentNoiseReductionDb.store(0.0f, std::memory_order_relaxed);
        currentVoiceProbability.store(0.0f, std::memory_order_relaxed);
        return;
    }

    const int numChannels = std::min(buffer.getNumChannels(), static_cast<int>(channels.size()));
    const int numSamples = buffer.getNumSamples();

    if (numSamples <= 0 || numChannels <= 0)
        return;

    for (int ch = 0; ch < numChannels; ++ch)
    {
        float* channelData = buffer.getWritePointer(ch);
        auto& state = channels[static_cast<size_t>(ch)];

        for (int i = 0; i < numSamples; ++i)
        {
            state.inputFifo[static_cast<size_t>(state.fifoIndex)] = channelData[i];

            // Extract output from accumulator
            channelData[i] = state.outputAccum[static_cast<size_t>(state.fifoIndex)];
            state.outputAccum[static_cast<size_t>(state.fifoIndex)] = 0.0f;

            state.fifoIndex++;
            if (state.fifoIndex >= HOP_SIZE)
            {
                // Process 512-sample frame with Overlap-Add
                processFrame(state, state.inputFifo.data());

                // Shift input buffer by HOP_SIZE
                std::memmove(state.inputFifo.data(), state.inputFifo.data() + HOP_SIZE, (FFT_SIZE - HOP_SIZE) * sizeof(float));
                std::memset(state.inputFifo.data() + (FFT_SIZE - HOP_SIZE), 0, HOP_SIZE * sizeof(float));

                state.fifoIndex = 0;
            }
        }
    }
}

void AiNoiseSuppressor::processFrame(ChannelState& ch, float* /*inOutTimeDomain*/)
{
    std::array<float, FFT_SIZE * 2> fftBuffer{};
    std::memcpy(fftBuffer.data(), ch.inputFifo.data(), FFT_SIZE * sizeof(float));

    // 1. Apply Analysis Window
    window.multiplyWithWindowingTable(fftBuffer.data(), FFT_SIZE);

    // 2. Forward FFT
    fft.performRealOnlyForwardTransform(fftBuffer.data());

    // 3. Compute Spectral Power in 24 Bark Bands
    std::array<float, NUM_BANDS> bandPower{};
    float totalFrameEnergy = 0.000001f;

    for (int b = 0; b < NUM_BANDS; ++b)
    {
        int startBin = bandBoundaries[static_cast<size_t>(b)];
        int endBin = bandBoundaries[static_cast<size_t>(b + 1)];
        int count = std::max(1, endBin - startBin);

        float pSum = 0.0f;
        for (int bin = startBin; bin < endBin; ++bin)
        {
            float re = fftBuffer[static_cast<size_t>(bin * 2)];
            float im = fftBuffer[static_cast<size_t>(bin * 2 + 1)];
            pSum += (re * re + im * im);
        }

        bandPower[static_cast<size_t>(b)] = pSum / static_cast<float>(count);
        totalFrameEnergy += pSum;
    }

    // 4. Neural VAD & Energy Tracker (GRU-Style Adaptive Estimation)
    ch.vadEnergyTracker = 0.85f * ch.vadEnergyTracker + 0.15f * totalFrameEnergy;
    if (totalFrameEnergy < ch.vadNoiseFloor * 1.5f || ch.vadNoiseFloor < 0.00001f)
    {
        ch.vadNoiseFloor = 0.96f * ch.vadNoiseFloor + 0.04f * totalFrameEnergy;
    }
    else
    {
        ch.vadNoiseFloor = 0.999f * ch.vadNoiseFloor + 0.001f * totalFrameEnergy;
    }

    // Calculate Speech Probability P(Speech) based on SNR and voice formant band concentration
    float voiceFormantEnergy = 0.0f;
    for (int b = 4; b <= 15; ++b) // 200Hz to 3.5kHz fundamental voice formants
    {
        voiceFormantEnergy += bandPower[static_cast<size_t>(b)];
    }

    float snrLinear = (voiceFormantEnergy + 0.00001f) / (ch.vadNoiseFloor * 8.0f + 0.00001f);
    float targetSpeechProb = juce::jlimit(0.0f, 1.0f, (snrLinear - 1.2f) / 4.0f);
    ch.speechProbability = 0.8f * ch.speechProbability + 0.2f * targetSpeechProb;
    currentVoiceProbability.store(ch.speechProbability, std::memory_order_relaxed);

    // 5. Update Recurrent Noise PSD & Late Reverberation Tracking
    const float alphaDenoise = denoiseAmount.load(std::memory_order_relaxed);
    const bool deReverbOn = deReverbEnabled.load(std::memory_order_relaxed);
    const float alphaDeReverb = deReverbAmount.load(std::memory_order_relaxed);

    float sumRawEnergy = 0.00001f;
    float sumFilteredEnergy = 0.00001f;

    std::array<float, NUM_BANDS> targetGains{};

    for (int b = 0; b < NUM_BANDS; ++b)
    {
        auto idx = static_cast<size_t>(b);
        float p = bandPower[idx];
        sumRawEnergy += p;

        // Noise estimation update rate is slower during speech, fast during silence
        float noiseAdaptRate = (ch.speechProbability < 0.25f) ? 0.08f : 0.002f;
        ch.noisePsd[idx] = (1.0f - noiseAdaptRate) * ch.noisePsd[idx] + noiseAdaptRate * p;

        // Late Reverb Diffuse Energy Decay Modeling
        float reverbDecayRate = 0.85f;
        ch.lateReverbPsd[idx] = reverbDecayRate * ch.lateReverbPsd[idx] + (1.0f - reverbDecayRate) * ch.speechPsd[idx];
        ch.speechPsd[idx] = 0.7f * ch.speechPsd[idx] + 0.3f * p;

        // Total Interference = Noise PSD + Late Reverb PSD
        float noiseEstimate = ch.noisePsd[idx];
        if (deReverbOn && alphaDeReverb > 0.01f)
        {
            noiseEstimate += ch.lateReverbPsd[idx] * (alphaDeReverb * 0.75f);
        }

        // Wiener / MMSE Spectral Gain Calculation
        float bandSnr = (p + 0.00001f) / (noiseEstimate + 0.00001f);
        float rawGain = std::max(0.0f, (bandSnr - 1.0f) / bandSnr);

        // Scale by denoise amount and apply soft knee
        float scaledGain = (1.0f - alphaDenoise) + (alphaDenoise * rawGain);

        // Protect voice formants (Speech Clarity Guard) so vocal is never muffled
        if (b >= 5 && b <= 15 && ch.speechProbability > 0.4f)
        {
            scaledGain = std::max(scaledGain, 0.45f);
        }

        // Minimum floor to avoid absolute zero gating artifacts
        float minGainFloor = (1.0f - alphaDenoise) * 0.15f + 0.03f;
        targetGains[idx] = juce::jlimit(minGainFloor, 1.0f, scaledGain);

        // Smooth gains across frames
        ch.smoothGains[idx] = 0.65f * ch.smoothGains[idx] + 0.35f * targetGains[idx];
        sumFilteredEnergy += p * (ch.smoothGains[idx] * ch.smoothGains[idx]);
    }

    // Measure Noise Reduction in dB
    float reductionRatio = std::sqrt(sumFilteredEnergy / sumRawEnergy);
    float reductionDb = 20.0f * std::log10(std::max(0.001f, reductionRatio));
    currentNoiseReductionDb.store(reductionDb, std::memory_order_relaxed);

    // 6. Apply Filterbank Gains to FFT Bins
    for (int b = 0; b < NUM_BANDS; ++b)
    {
        int startBin = bandBoundaries[static_cast<size_t>(b)];
        int endBin = bandBoundaries[static_cast<size_t>(b + 1)];
        float gain = ch.smoothGains[static_cast<size_t>(b)];

        for (int bin = startBin; bin < endBin; ++bin)
        {
            size_t reIdx = static_cast<size_t>(bin * 2);
            size_t imIdx = static_cast<size_t>(bin * 2 + 1);

            fftBuffer[reIdx] *= gain;
            fftBuffer[imIdx] *= gain;
        }
    }

    // 7. Inverse FFT
    fft.performRealOnlyInverseTransform(fftBuffer.data());

    // 8. Synthesis Window & Overlap-Add
    window.multiplyWithWindowingTable(fftBuffer.data(), FFT_SIZE);

    // Scaling factor for 75% overlap (HOP_SIZE = FFT_SIZE / 4) with Hann window
    const float olaScale = 2.0f / 3.0f;
    for (int i = 0; i < FFT_SIZE; ++i)
    {
        ch.outputAccum[static_cast<size_t>(i)] += fftBuffer[static_cast<size_t>(i)] * olaScale;
    }
}
