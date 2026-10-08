#include "AiNoiseSuppressor.h"
#include <algorithm>

AiNoiseSuppressor::AiNoiseSuppressor()
{
    channels.resize(2);
    calculateCoefficients();
    reset();
}

void AiNoiseSuppressor::calculateCoefficients()
{
    // 16 Bark/ERB Critical Band Center Frequencies
    const float centerFreqs[NUM_BANDS] = {
        65.0f,   130.0f,  220.0f,  350.0f, 
        520.0f,  750.0f,  1100.0f, 1600.0f, 
        2300.0f, 3300.0f, 4700.0f, 6600.0f, 
        9200.0f, 12500.0f, 16000.0f, 19500.0f
    };

    const float Q = 1.35f;
    const double pi = juce::MathConstants<double>::pi;

    for (int k = 0; k < NUM_BANDS; ++k)
    {
        double fc = std::clamp(static_cast<double>(centerFreqs[k]), 20.0, currentSampleRate * 0.46);
        double w0 = 2.0 * pi * fc / currentSampleRate;
        double alpha = std::sin(w0) / (2.0 * static_cast<double>(Q));

        double a0 = 1.0 + alpha;
        bandFilters[static_cast<size_t>(k)].b0 = static_cast<float>(alpha / a0);
        bandFilters[static_cast<size_t>(k)].b1 = 0.0f;
        bandFilters[static_cast<size_t>(k)].b2 = static_cast<float>(-alpha / a0);
        bandFilters[static_cast<size_t>(k)].a1 = static_cast<float>((-2.0 * std::cos(w0)) / a0);
        bandFilters[static_cast<size_t>(k)].a2 = static_cast<float>((1.0 - alpha) / a0);
    }
}

void AiNoiseSuppressor::prepare(double sampleRate, int /*samplesPerBlock*/)
{
    currentSampleRate = (sampleRate > 0.0) ? sampleRate : 44100.0;
    channels.resize(2);
    calculateCoefficients();
    reset();
}

void AiNoiseSuppressor::reset()
{
    for (auto& ch : channels)
    {
        for (auto& bState : ch.biquadStates)
        {
            bState.s1 = 0.0f;
            bState.s2 = 0.0f;
        }

        ch.envelope.fill(0.0001f);
        ch.noiseFloor.fill(0.0001f);
        ch.smoothGains.fill(1.0f);
        ch.reverbTail.fill(0.0001f);

        ch.speechProbability = 0.0f;
        ch.shortTermInRms = 0.0001f;
        ch.shortTermOutRms = 0.0001f;
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

    const float alphaDenoise = denoiseAmount.load(std::memory_order_relaxed);
    const bool deReverbOn = deReverbEnabled.load(std::memory_order_relaxed);
    const float alphaDeReverb = deReverbAmount.load(std::memory_order_relaxed);

    float sumInEnergy = 0.000001f;
    float sumOutEnergy = 0.000001f;
    float avgVoiceProb = 0.0f;

    for (int c = 0; c < numChannels; ++c)
    {
        float* channelData = buffer.getWritePointer(c);
        auto& ch = channels[static_cast<size_t>(c)];

        for (int i = 0; i < numSamples; ++i)
        {
            const float x = channelData[i];
            float sumNoiseSub = 0.0f;
            std::array<float, NUM_BANDS> bandOuts{};

            // 1. Split into 16 Critical Bands via Direct Form II Transposed Biquads
            for (int k = 0; k < NUM_BANDS; ++k)
            {
                auto idx = static_cast<size_t>(k);
                const auto& coeffs = bandFilters[idx];
                auto& state = ch.biquadStates[idx];

                // Direct Form II Transposed
                float y = coeffs.b0 * x + state.s1;
                state.s1 = coeffs.b1 * x - coeffs.a1 * y + state.s2;
                state.s2 = coeffs.b2 * x - coeffs.a2 * y;

                bandOuts[idx] = y;

                // Fast Envelope Detector
                float absY = std::abs(y);
                float envAlpha = (absY > ch.envelope[idx]) ? 0.85f : 0.985f;
                ch.envelope[idx] = envAlpha * ch.envelope[idx] + (1.0f - envAlpha) * absY;
            }

            // 2. Compute Voice Formant Energy vs Noise Bands
            float vocalEnergy = 0.00001f;
            for (int k = 3; k <= 9; ++k) // 350 Hz to 3.3 kHz
            {
                vocalEnergy += ch.envelope[static_cast<size_t>(k)];
            }

            float noiseBandsEnergy = 0.00001f;
            noiseBandsEnergy += ch.envelope[0]; // 65 Hz
            noiseBandsEnergy += ch.envelope[1]; // 130 Hz
            noiseBandsEnergy += ch.envelope[11]; // 6.6 kHz
            noiseBandsEnergy += ch.envelope[12]; // 9.2 kHz
            noiseBandsEnergy += ch.envelope[13]; // 12.5 kHz
            noiseBandsEnergy += ch.envelope[14]; // 16 kHz
            noiseBandsEnergy += ch.envelope[15]; // 19.5 kHz

            // 3. Neural-style VAD Voice Probability
            float instantaneousSnr = vocalEnergy / (noiseBandsEnergy * 1.5f + 0.00001f);
            float targetVoiceProb = juce::jlimit(0.0f, 1.0f, (instantaneousSnr - 0.9f) / 2.5f);
            ch.speechProbability = 0.990f * ch.speechProbability + 0.010f * targetVoiceProb;

            // 4. Update Noise Floor & Compute Multi-Band Gains
            for (int k = 0; k < NUM_BANDS; ++k)
            {
                auto idx = static_cast<size_t>(k);
                float env = ch.envelope[idx];

                // Adaptive Noise Floor Tracking
                float nfRate = (ch.speechProbability < 0.30f) ? 0.003f : 0.00005f;
                if (env < ch.noiseFloor[idx] * 1.2f)
                {
                    ch.noiseFloor[idx] = 0.992f * ch.noiseFloor[idx] + 0.008f * env;
                }
                else
                {
                    ch.noiseFloor[idx] = (1.0f - nfRate) * ch.noiseFloor[idx] + nfRate * env;
                }

                // Band SNR
                float bandSnr = env / (ch.noiseFloor[idx] + 0.000001f);

                // Multi-Band Downward Expander / Spectral Subtraction Gain
                float rawGain = juce::jlimit(0.0f, 1.0f, (bandSnr - 1.05f) / std::max(0.2f, bandSnr));
                float targetGain = (1.0f - alphaDenoise) + (alphaDenoise * rawGain);

                // Speech Formant Clarity Guard (Preserve natural vocal body & consonants)
                if (k >= 4 && k <= 9 && ch.speechProbability > 0.35f)
                {
                    targetGain = std::max(targetGain, 0.58f);
                }

                // Room De-Reverb: Suppress diffuse reverberation tail when voice stops
                if (deReverbOn && alphaDeReverb > 0.01f && k >= 2 && k <= 8)
                {
                    if (ch.speechProbability < 0.25f && bandSnr > 1.3f)
                    {
                        targetGain *= (1.0f - alphaDeReverb * 0.45f);
                    }
                }

                targetGain = juce::jlimit(0.06f, 1.0f, targetGain);

                // Sample-by-sample smooth gain ramp (Zero clicks / pops)
                float gainRamp = (targetGain < ch.smoothGains[idx]) ? 0.995f : 0.988f;
                ch.smoothGains[idx] = gainRamp * ch.smoothGains[idx] + (1.0f - gainRamp) * targetGain;

                // Accumulate noise component to subtract
                sumNoiseSub += bandOuts[idx] * (1.0f - ch.smoothGains[idx]);
            }

            // 5. Subtract Noise Component (Weighted for linear transparency)
            float outSample = x - sumNoiseSub * 0.50f;

            // Output limiter guard to prevent digital clipping
            outSample = juce::jlimit(-1.0f, 1.0f, outSample);

            channelData[i] = outSample;

            sumInEnergy += x * x;
            sumOutEnergy += outSample * outSample;
        }

        avgVoiceProb += ch.speechProbability;
    }

    avgVoiceProb /= static_cast<float>(numChannels);
    currentVoiceProbability.store(avgVoiceProb, std::memory_order_relaxed);

    // Measure dB Reduction
    float ratio = std::sqrt(sumOutEnergy / sumInEnergy);
    float reductionDb = 20.0f * std::log10(std::max(0.001f, ratio));
    currentNoiseReductionDb.store(reductionDb, std::memory_order_relaxed);
}

