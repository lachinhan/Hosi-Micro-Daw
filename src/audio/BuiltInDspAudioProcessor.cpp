#include "BuiltInDspAudioProcessor.h"
#include <cmath>
#include <algorithm>

BuiltInDspAudioProcessor::BuiltInDspAudioProcessor()
    : AudioProcessor(BusesProperties().withInput("Input", juce::AudioChannelSet::stereo(), true)
                                      .withOutput("Output", juce::AudioChannelSet::stereo(), true))
{
    loadPreset(VocalPreset::BypassAll);
}

void BuiltInDspAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    currentSampleRate = (sampleRate > 8000.0) ? sampleRate : 44100.0;

    // AI Noise Suppressor & Room De-Reverb Prepare
    aiNoiseSuppressor.prepare(currentSampleRate, samplesPerBlock);
    aiVocalProfiler.prepare(currentSampleRate, samplesPerBlock);

    // Pre-allocate temp reverb buffer to eliminate heap allocation on audio thread
    tempReverbBuffer.setSize(2, std::max(samplesPerBlock, 2048));

    // Noise Gate Initial State
    currentGateGain = 1.0f;
    gateEnvelope = 0.0f;
    gateStateOpen = true;
    gateHoldSamplesRemaining = 0;
    gateIsOpen.store(true, std::memory_order_release);

    // Compressor Initial State
    currentCompGain = 1.0f;
    compEnvelope = 0.0f;
    compGainReductionDb.store(0.0f, std::memory_order_release);

    // EQ Filters Reset
    lowShelfL.reset(); lowShelfR.reset();
    midPeakL.reset(); midPeakR.reset();
    highShelfL.reset(); highShelfR.reset();
    needEqUpdate.store(true, std::memory_order_release);
    updateEqCoefficients();

    // Reverb Reset
    reverbProcessor.setSampleRate(currentSampleRate);
    updateReverbParams();

    // Delay Buffer (Max 2.5 seconds)
    const int maxDelaySamples = static_cast<int>(currentSampleRate * 2.5) + 2048;
    delayBuffer.setSize(2, maxDelaySamples);
    delayBuffer.clear();
    delayWritePos = 0;
    delayLowPassL = 0.0f;
    delayLowPassR = 0.0f;

    const float initialTimeMs = getEffectiveDelayTimeMs();
    currentSmoothedDelaySamplesL = static_cast<float>(currentSampleRate * (initialTimeMs * 0.001f));
    currentSmoothedDelaySamplesR = static_cast<float>(currentSampleRate * (initialTimeMs * 0.00135f));

    limiterPeakEnv = 0.0f;
}

void BuiltInDspAudioProcessor::releaseResources()
{
    reverbProcessor.reset();
    delayBuffer.setSize(0, 0);
    tempReverbBuffer.setSize(0, 0);
}

void BuiltInDspAudioProcessor::updateEqCoefficients()
{
    if (currentSampleRate <= 0.0) return;

    const float lowDb = eqLowGainDb.load(std::memory_order_relaxed);
    const float midDb = eqMidGainDb.load(std::memory_order_relaxed);
    const float highDb = eqHighGainDb.load(std::memory_order_relaxed);

    const float lowGain = juce::Decibels::decibelsToGain(lowDb);
    const float midGain = juce::Decibels::decibelsToGain(midDb);
    const float highGain = juce::Decibels::decibelsToGain(highDb);

    auto lowCoeffs = juce::dsp::IIR::Coefficients<float>::makeLowShelf(currentSampleRate, 120.0, 0.707, lowGain);
    auto midCoeffs = juce::dsp::IIR::Coefficients<float>::makePeakFilter(currentSampleRate, 2600.0, 1.0, midGain);
    auto highCoeffs = juce::dsp::IIR::Coefficients<float>::makeHighShelf(currentSampleRate, 9500.0, 0.707, highGain);

    lowShelfL.coefficients = lowCoeffs;
    lowShelfR.coefficients = lowCoeffs;
    midPeakL.coefficients = midCoeffs;
    midPeakR.coefficients = midCoeffs;
    highShelfL.coefficients = highCoeffs;
    highShelfR.coefficients = highCoeffs;

    needEqUpdate.store(false, std::memory_order_release);
}

void BuiltInDspAudioProcessor::updateReverbParams()
{
    if (reverbBpmSync.load(std::memory_order_relaxed))
    {
        const double bpm = hostBpm.load(std::memory_order_relaxed);
        const auto barLen = reverbBarLength.load(std::memory_order_relaxed);
        const float decaySec = TempoSyncEngine::calculateReverbDecaySec(bpm, barLen);

        // Map decay seconds (0.5s - 8.0s) smoothly into juce::Reverb room size (0.30 - 0.94)
        const float autoSize = std::clamp(std::sqrt(decaySec / 6.5f) * 0.85f, 0.28f, 0.94f);
        const float autoDamp = 0.32f; // Keep top air crisp

        reverbParams.roomSize = autoSize;
        reverbParams.damping = autoDamp;
    }
    else
    {
        reverbParams.roomSize = std::clamp(reverbSize.load(std::memory_order_relaxed), 0.0f, 1.0f);
        reverbParams.damping = std::clamp(reverbDamp.load(std::memory_order_relaxed), 0.0f, 1.0f);
    }

    reverbParams.wetLevel = 1.0f; // Managed via wetMix blending
    reverbParams.dryLevel = 0.0f;
    reverbParams.width = 1.0f;
    reverbParams.freezeMode = 0.0f;
    reverbProcessor.setParameters(reverbParams);
}

void BuiltInDspAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();
    if (numSamples <= 0 || numChannels <= 0)
        return;

    const bool hasAi = aiNoiseSuppressor.isEnabled();
    const bool hasGate = gateEnabled.load(std::memory_order_relaxed);
    const bool hasEq = eqEnabled.load(std::memory_order_relaxed);
    const bool hasComp = compEnabled.load(std::memory_order_relaxed);
    const bool hasDelay = delayEnabled.load(std::memory_order_relaxed);
    const bool hasReverb = reverbEnabled.load(std::memory_order_relaxed);
    const bool hasLimiter = limiterEnabled.load(std::memory_order_relaxed);

    // AI Vocal Profiler Input Feed (If actively recording sample)
    if (aiVocalProfiler.isProfiling())
    {
        aiVocalProfiler.processBlock(buffer);
    }

    // If completely bypassed, return immediately with 0 overhead
    if (!hasAi && !hasGate && !hasEq && !hasComp && !hasDelay && !hasReverb && !hasLimiter)
        return;

    // -------------------------------------------------------------
    // 0. AI REAL-TIME NOISE SUPPRESSOR & ROOM DE-REVERB SHIELD
    // -------------------------------------------------------------
    if (hasAi)
    {
        aiNoiseSuppressor.process(buffer);
    }

    // Check EQ coefficient updates
    if (hasEq && needEqUpdate.load(std::memory_order_relaxed))
    {
        updateEqCoefficients();
    }

    float* left = buffer.getWritePointer(0);
    float* right = (numChannels > 1) ? buffer.getWritePointer(1) : left;

    // -------------------------------------------------------------
    // 1. NOISE GATE (Fast Attack, Smooth Release, Hysteresis & Hold)
    // -------------------------------------------------------------
    if (hasGate)
    {
        const float threshDb = gateThresholdDb.load(std::memory_order_relaxed);
        const float openThreshLin = juce::Decibels::decibelsToGain(threshDb);
        const float closeThreshLin = openThreshLin * 0.63f; // ~ -4 dB hysteresis

        const float gateAttackAlpha = static_cast<float>(std::exp(-1.0 / (currentSampleRate * 0.0015))); // 1.5ms
        const float gateReleaseAlpha = static_cast<float>(std::exp(-1.0 / (currentSampleRate * 0.120))); // 120ms
        const float gateGainAlpha = static_cast<float>(std::exp(-1.0 / (currentSampleRate * 0.006)));    // 6ms ramp
        const int holdSamples = static_cast<int>(currentSampleRate * 0.035); // 35ms hold time

        for (int i = 0; i < numSamples; ++i)
        {
            const float inputPeak = std::max(std::abs(left[i]), std::abs(right[i]));

            if (inputPeak > gateEnvelope)
                gateEnvelope = gateAttackAlpha * gateEnvelope + (1.0f - gateAttackAlpha) * inputPeak;
            else
                gateEnvelope = gateReleaseAlpha * gateEnvelope + (1.0f - gateReleaseAlpha) * inputPeak;

            if (gateStateOpen)
            {
                if (gateEnvelope < closeThreshLin)
                {
                    if (gateHoldSamplesRemaining > 0)
                        --gateHoldSamplesRemaining;
                    else
                        gateStateOpen = false;
                }
                else
                {
                    gateHoldSamplesRemaining = holdSamples;
                }
            }
            else
            {
                if (gateEnvelope > openThreshLin)
                {
                    gateStateOpen = true;
                    gateHoldSamplesRemaining = holdSamples;
                }
            }

            const float targetGain = gateStateOpen ? 1.0f : 0.0f;
            currentGateGain = gateGainAlpha * currentGateGain + (1.0f - gateGainAlpha) * targetGain;

            left[i] *= currentGateGain;
            right[i] *= currentGateGain;
        }

        gateIsOpen.store(gateStateOpen, std::memory_order_relaxed);
    }
    else
    {
        currentGateGain = 1.0f;
        gateStateOpen = true;
        gateIsOpen.store(true, std::memory_order_relaxed);
    }

    // -------------------------------------------------------------
    // 2. STUDIO 3-BAND EQ (Low Shelf, Mid Presence, High Air)
    // -------------------------------------------------------------
    if (eqEnabled.load(std::memory_order_relaxed))
    {
        for (int i = 0; i < numSamples; ++i)
        {
            left[i] = lowShelfL.processSample(left[i]);
            left[i] = midPeakL.processSample(left[i]);
            left[i] = highShelfL.processSample(left[i]);

            if (numChannels > 1)
            {
                right[i] = lowShelfR.processSample(right[i]);
                right[i] = midPeakR.processSample(right[i]);
                right[i] = highShelfR.processSample(right[i]);
            }
        }
    }

    // -------------------------------------------------------------
    // 3. WARM VOCAL COMPRESSOR (VCA/Optical curve + Makeup)
    // -------------------------------------------------------------
    if (compEnabled.load(std::memory_order_relaxed))
    {
        const float threshDb = compThresholdDb.load(std::memory_order_relaxed);
        const float ratio = std::max(1.0f, compRatio.load(std::memory_order_relaxed));
        const float makeupLin = juce::Decibels::decibelsToGain(compMakeupDb.load(std::memory_order_relaxed));

        const float compAttackAlpha = static_cast<float>(std::exp(-1.0 / (currentSampleRate * 0.012))); // 12ms
        const float compReleaseAlpha = static_cast<float>(std::exp(-1.0 / (currentSampleRate * 0.085))); // 85ms
        const float compGainAlpha = static_cast<float>(std::exp(-1.0 / (currentSampleRate * 0.005)));   // 5ms smooth

        float maxReductionDb = 0.0f;

        for (int i = 0; i < numSamples; ++i)
        {
            const float peak = std::max(std::abs(left[i]), std::abs(right[i]));

            if (peak > compEnvelope)
                compEnvelope = compAttackAlpha * compEnvelope + (1.0f - compAttackAlpha) * peak;
            else
                compEnvelope = compReleaseAlpha * compEnvelope + (1.0f - compReleaseAlpha) * peak;

            const float envDb = juce::Decibels::gainToDecibels(std::max(0.00001f, compEnvelope));

            float gainReductionDb = 0.0f;
            if (envDb > threshDb)
            {
                const float overDb = envDb - threshDb;
                gainReductionDb = overDb * (1.0f - 1.0f / ratio);
            }

            maxReductionDb = std::max(maxReductionDb, gainReductionDb);

            const float targetCompGain = juce::Decibels::decibelsToGain(-gainReductionDb) * makeupLin;
            currentCompGain = compGainAlpha * currentCompGain + (1.0f - compGainAlpha) * targetCompGain;

            left[i] *= currentCompGain;
            right[i] *= currentCompGain;

            // Smooth cubic warm saturation (mild soft clipping without std::tanh overhead)
            const float l = left[i] * 0.95f;
            const float r = right[i] * 0.95f;
            left[i] = (std::abs(l) < 1.0f) ? (l - (l * l * l * 0.15f)) : (l > 0.0f ? 0.85f : -0.85f);
            right[i] = (std::abs(r) < 1.0f) ? (r - (r * r * r * 0.15f)) : (r > 0.0f ? 0.85f : -0.85f);
        }

        compGainReductionDb.store(maxReductionDb, std::memory_order_relaxed);
    }
    else
    {
        currentCompGain = 1.0f;
        compGainReductionDb.store(0.0f, std::memory_order_relaxed);
    }

    // -------------------------------------------------------------
    // 4. STEREO DELAY / ECHO (Smart BPM Subdivisions + Ping-Pong)
    // -------------------------------------------------------------
    if (delayEnabled.load(std::memory_order_relaxed) && delayBuffer.getNumSamples() > 0)
    {
        const float wet = delayWetMix.load(std::memory_order_relaxed);
        const float fb = std::clamp(delayFeedback.load(std::memory_order_relaxed), 0.0f, 0.85f);
        const float targetTimeMs = getEffectiveDelayTimeMs();

        const float targetSamplesL = std::clamp(static_cast<float>(currentSampleRate * (targetTimeMs * 0.001f)), 1.0f, static_cast<float>(delayBuffer.getNumSamples() - 2));
        const float targetSamplesR = std::clamp(static_cast<float>(currentSampleRate * (targetTimeMs * 0.00135f)), 1.0f, static_cast<float>(delayBuffer.getNumSamples() - 2));

        const int bufferSize = delayBuffer.getNumSamples();
        float* dL = delayBuffer.getWritePointer(0);
        float* dR = delayBuffer.getWritePointer(1);

        const float lpfAlpha = 0.28f; // ~4.5kHz analog tape high cut
        const float smoothAlpha = 0.004f; // Smooth tape head inertia for seamless tempo adjustments

        for (int i = 0; i < numSamples; ++i)
        {
            currentSmoothedDelaySamplesL = currentSmoothedDelaySamplesL * (1.0f - smoothAlpha) + targetSamplesL * smoothAlpha;
            currentSmoothedDelaySamplesR = currentSmoothedDelaySamplesR * (1.0f - smoothAlpha) + targetSamplesR * smoothAlpha;

            const float inL = left[i];
            const float inR = right[i];

            // Linear interpolated fractional delay reading
            const float readPosFloatL = static_cast<float>(delayWritePos) - currentSmoothedDelaySamplesL + static_cast<float>(bufferSize * 2);
            const int readPosIntL = static_cast<int>(readPosFloatL) % bufferSize;
            const int nextPosL = (readPosIntL + 1) % bufferSize;
            const float fracL = readPosFloatL - std::floor(readPosFloatL);
            const float delayedL = dL[readPosIntL] * (1.0f - fracL) + dL[nextPosL] * fracL;

            const float readPosFloatR = static_cast<float>(delayWritePos) - currentSmoothedDelaySamplesR + static_cast<float>(bufferSize * 2);
            const int readPosIntR = static_cast<int>(readPosFloatR) % bufferSize;
            const int nextPosR = (readPosIntR + 1) % bufferSize;
            const float fracR = readPosFloatR - std::floor(readPosFloatR);
            const float delayedR = dR[readPosIntR] * (1.0f - fracR) + dR[nextPosR] * fracR;

            // Low-pass filter on feedback path
            delayLowPassL = delayLowPassL * (1.0f - lpfAlpha) + delayedL * lpfAlpha;
            delayLowPassR = delayLowPassR * (1.0f - lpfAlpha) + delayedR * lpfAlpha;

            // Ping-pong cross feedback into delay buffer
            dL[delayWritePos] = inL + delayLowPassR * fb;
            dR[delayWritePos] = inR + delayLowPassL * fb;

            delayWritePos = (delayWritePos + 1) % bufferSize;

            // Mix wet delay signal
            left[i] += delayedL * wet;
            right[i] += delayedR * wet;
        }
    }

    // -------------------------------------------------------------
    // 5. LUSH STUDIO REVERB (Smart BPM Auto-Tail Space)
    // -------------------------------------------------------------
    if (hasReverb)
    {
        const float wet = reverbWetMix.load(std::memory_order_relaxed);

        if (numChannels >= 2)
        {
            if (tempReverbBuffer.getNumSamples() < numSamples)
                tempReverbBuffer.setSize(2, numSamples, false, false, true);

            tempReverbBuffer.copyFrom(0, 0, left, numSamples);
            tempReverbBuffer.copyFrom(1, 0, right, numSamples);

            reverbProcessor.processStereo(tempReverbBuffer.getWritePointer(0), tempReverbBuffer.getWritePointer(1), numSamples);

            for (int i = 0; i < numSamples; ++i)
            {
                left[i] = left[i] * (1.0f - wet * 0.4f) + tempReverbBuffer.getSample(0, i) * wet;
                right[i] = right[i] * (1.0f - wet * 0.4f) + tempReverbBuffer.getSample(1, i) * wet;
            }
        }
        else
        {
            reverbProcessor.processMono(left, numSamples);
        }
    }

    // -------------------------------------------------------------
    // 6. BRICKWALL PEAK LIMITER (-0.3 dBFS Anti-Clipping)
    // -------------------------------------------------------------
    if (hasLimiter)
    {
        const float threshDb = limiterThresholdDb.load(std::memory_order_relaxed);
        const float ceilingLin = juce::Decibels::decibelsToGain(threshDb);
        const float limiterRelease = static_cast<float>(std::exp(-1.0 / (currentSampleRate * 0.040))); // 40ms

        for (int i = 0; i < numSamples; ++i)
        {
            const float peak = std::max(std::abs(left[i]), std::abs(right[i]));

            if (peak > limiterPeakEnv)
                limiterPeakEnv = peak;
            else
                limiterPeakEnv = limiterRelease * limiterPeakEnv + (1.0f - limiterRelease) * peak;

            if (limiterPeakEnv > ceilingLin)
            {
                const float reduction = ceilingLin / limiterPeakEnv;
                left[i] *= reduction;
                right[i] *= reduction;
            }

            // Hard safety clamp at ±0.999f
            left[i] = std::clamp(left[i], -0.999f, 0.999f);
            right[i] = std::clamp(right[i], -0.999f, 0.999f);
        }
    }
}

void BuiltInDspAudioProcessor::loadPreset(VocalPreset preset)
{
    currentPreset.store(preset, std::memory_order_release);

    // Ensure seamless preset switching without audio cutout
    currentGateGain = 1.0f;
    gateStateOpen = true;
    gateHoldSamplesRemaining = static_cast<int>(currentSampleRate * 0.035);
    gateIsOpen.store(true, std::memory_order_release);
    currentCompGain = 1.0f;
    compGainReductionDb.store(0.0f, std::memory_order_release);

    switch (preset)
    {
    case VocalPreset::LiveSinging:
        setAiDenoiseEnabled(true);
        setAiDenoiseAmount(0.75f);
        setAiDeReverbEnabled(true);
        setAiDeReverbAmount(0.40f);
        setGateEnabled(true);
        setGateThresholdDb(-48.0f);
        setEqEnabled(true);
        setEqLowGainDb(-1.0f);
        setEqMidGainDb(2.5f);
        setEqHighGainDb(3.0f);
        setCompEnabled(true);
        setCompThresholdDb(-18.0f);
        setCompRatio(3.5f);
        setCompMakeupDb(3.0f);
        setReverbEnabled(true);
        setReverbBpmSync(true);
        setReverbBarLength(TempoSyncEngine::ReverbBarLength::OneBar);
        setReverbWetMix(0.26f);
        setDelayEnabled(true);
        setDelayBpmSync(true);
        setDelaySubdivision(TempoSyncEngine::DelaySubdivision::DottedEighth);
        setDelayFeedback(0.28f);
        setDelayWetMix(0.18f);
        setLimiterEnabled(true);
        setLimiterThresholdDb(-0.5f);
        break;

    case VocalPreset::StreamerTalk:
        setAiDenoiseEnabled(true);
        setAiDenoiseAmount(0.85f);
        setAiDeReverbEnabled(true);
        setAiDeReverbAmount(0.50f);
        setGateEnabled(true);
        setGateThresholdDb(-44.0f);
        setEqEnabled(true);
        setEqLowGainDb(1.0f);
        setEqMidGainDb(3.0f);
        setEqHighGainDb(2.0f);
        setCompEnabled(true);
        setCompThresholdDb(-15.0f);
        setCompRatio(4.0f);
        setCompMakeupDb(2.0f);
        setReverbEnabled(true);
        setReverbBpmSync(false);
        setReverbSize(0.40f);
        setReverbDamp(0.50f);
        setReverbWetMix(0.08f);
        setDelayEnabled(false);
        setLimiterEnabled(true);
        setLimiterThresholdDb(-0.5f);
        break;

    case VocalPreset::KaraokeHall:
        setAiDenoiseEnabled(true);
        setAiDenoiseAmount(0.70f);
        setAiDeReverbEnabled(true);
        setAiDeReverbAmount(0.30f);
        setGateEnabled(true);
        setGateThresholdDb(-50.0f);
        setEqEnabled(true);
        setEqLowGainDb(0.0f);
        setEqMidGainDb(2.0f);
        setEqHighGainDb(3.5f);
        setCompEnabled(true);
        setCompThresholdDb(-20.0f);
        setCompRatio(3.0f);
        setCompMakeupDb(2.5f);
        setReverbEnabled(true);
        setReverbBpmSync(true);
        setReverbBarLength(TempoSyncEngine::ReverbBarLength::TwoBars);
        setReverbWetMix(0.35f);
        setDelayEnabled(true);
        setDelayBpmSync(true);
        setDelaySubdivision(TempoSyncEngine::DelaySubdivision::Quarter);
        setDelayFeedback(0.35f);
        setDelayWetMix(0.24f);
        setLimiterEnabled(true);
        setLimiterThresholdDb(-0.5f);
        break;

    case VocalPreset::PodcastClean:
        setAiDenoiseEnabled(true);
        setAiDenoiseAmount(0.90f);
        setAiDeReverbEnabled(true);
        setAiDeReverbAmount(0.60f);
        setGateEnabled(true);
        setGateThresholdDb(-46.0f);
        setEqEnabled(true);
        setEqLowGainDb(-2.0f);
        setEqMidGainDb(1.5f);
        setEqHighGainDb(1.5f);
        setCompEnabled(true);
        setCompThresholdDb(-16.0f);
        setCompRatio(3.0f);
        setCompMakeupDb(2.0f);
        setReverbEnabled(false);
        setDelayEnabled(false);
        setLimiterEnabled(true);
        setLimiterThresholdDb(-0.5f);
        break;

    case VocalPreset::BypassAll:
        setAiDenoiseEnabled(false);
        setAiDeReverbEnabled(false);
        setGateEnabled(false);
        setEqEnabled(false);
        setCompEnabled(false);
        setReverbEnabled(false);
        setDelayEnabled(false);
        setLimiterEnabled(false);
        break;

    case VocalPreset::FactoryReset:
        resetToFactoryDefaults();
        return;
    }

    sendChangeMessage();
}

void BuiltInDspAudioProcessor::resetToFactoryDefaults()
{
    loadPreset(VocalPreset::LiveSinging);
}

void BuiltInDspAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    juce::XmlElement xml("BuiltInDspState");
    xml.setAttribute("aiDenoiseEnabled", isAiDenoiseEnabled());
    xml.setAttribute("aiDenoiseAmount", getAiDenoiseAmount());
    xml.setAttribute("aiDeReverbEnabled", isAiDeReverbEnabled());
    xml.setAttribute("aiDeReverbAmount", getAiDeReverbAmount());
    xml.setAttribute("gateEnabled", isGateEnabled());
    xml.setAttribute("gateThresh", getGateThresholdDb());
    xml.setAttribute("eqEnabled", isEqEnabled());
    xml.setAttribute("eqLow", getEqLowGainDb());
    xml.setAttribute("eqMid", getEqMidGainDb());
    xml.setAttribute("eqHigh", getEqHighGainDb());
    xml.setAttribute("compEnabled", isCompEnabled());
    xml.setAttribute("compThresh", getCompThresholdDb());
    xml.setAttribute("compRatio", getCompRatio());
    xml.setAttribute("compMakeup", getCompMakeupDb());
    xml.setAttribute("reverbEnabled", isReverbEnabled());
    xml.setAttribute("reverbSize", getReverbSize());
    xml.setAttribute("reverbDamp", getReverbDamp());
    xml.setAttribute("reverbWet", getReverbWetMix());
    xml.setAttribute("reverbBpmSync", isReverbBpmSync());
    xml.setAttribute("reverbBarLength", static_cast<int>(getReverbBarLength()));
    xml.setAttribute("delayEnabled", isDelayEnabled());
    xml.setAttribute("delayTime", getDelayTimeMs());
    xml.setAttribute("delayFeedback", getDelayFeedback());
    xml.setAttribute("delayWet", getDelayWetMix());
    xml.setAttribute("delayBpmSync", isDelayBpmSync());
    xml.setAttribute("delaySubdivision", static_cast<int>(getDelaySubdivision()));
    xml.setAttribute("hostBpm", getHostBpm());
    xml.setAttribute("limiterEnabled", isLimiterEnabled());
    xml.setAttribute("preset", static_cast<int>(getCurrentPreset()));

    copyXmlToBinary(xml, destData);
}

void BuiltInDspAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    auto xml = getXmlFromBinary(data, sizeInBytes);
    if (xml != nullptr && xml->hasTagName("BuiltInDspState"))
    {
        setAiDenoiseEnabled(xml->getBoolAttribute("aiDenoiseEnabled", true));
        setAiDenoiseAmount(static_cast<float>(xml->getDoubleAttribute("aiDenoiseAmount", 0.75)));
        setAiDeReverbEnabled(xml->getBoolAttribute("aiDeReverbEnabled", true));
        setAiDeReverbAmount(static_cast<float>(xml->getDoubleAttribute("aiDeReverbAmount", 0.40)));
        setGateEnabled(xml->getBoolAttribute("gateEnabled", true));
        setGateThresholdDb(static_cast<float>(xml->getDoubleAttribute("gateThresh", -48.0)));
        setEqEnabled(xml->getBoolAttribute("eqEnabled", true));
        setEqLowGainDb(static_cast<float>(xml->getDoubleAttribute("eqLow", 0.0)));
        setEqMidGainDb(static_cast<float>(xml->getDoubleAttribute("eqMid", 2.0)));
        setEqHighGainDb(static_cast<float>(xml->getDoubleAttribute("eqHigh", 2.5)));
        setCompEnabled(xml->getBoolAttribute("compEnabled", true));
        setCompThresholdDb(static_cast<float>(xml->getDoubleAttribute("compThresh", -18.0)));
        setCompRatio(static_cast<float>(xml->getDoubleAttribute("compRatio", 3.2)));
        setCompMakeupDb(static_cast<float>(xml->getDoubleAttribute("compMakeup", 2.5)));
        setReverbEnabled(xml->getBoolAttribute("reverbEnabled", true));
        setReverbSize(static_cast<float>(xml->getDoubleAttribute("reverbSize", 0.65)));
        setReverbDamp(static_cast<float>(xml->getDoubleAttribute("reverbDamp", 0.35)));
        setReverbWetMix(static_cast<float>(xml->getDoubleAttribute("reverbWet", 0.22)));
        setReverbBpmSync(xml->getBoolAttribute("reverbBpmSync", true));
        setReverbBarLength(static_cast<TempoSyncEngine::ReverbBarLength>(xml->getIntAttribute("reverbBarLength", 1)));
        setDelayEnabled(xml->getBoolAttribute("delayEnabled", false));
        setDelayTimeMs(static_cast<float>(xml->getDoubleAttribute("delayTime", 260.0)));
        setDelayFeedback(static_cast<float>(xml->getDoubleAttribute("delayFeedback", 0.25)));
        setDelayWetMix(static_cast<float>(xml->getDoubleAttribute("delayWet", 0.18)));
        setDelayBpmSync(xml->getBoolAttribute("delayBpmSync", true));
        setDelaySubdivision(static_cast<TempoSyncEngine::DelaySubdivision>(xml->getIntAttribute("delaySubdivision", 1)));
        setHostBpm(xml->getDoubleAttribute("hostBpm", 120.0));
        setLimiterEnabled(xml->getBoolAttribute("limiterEnabled", true));
        currentPreset.store(static_cast<VocalPreset>(xml->getIntAttribute("preset", 0)), std::memory_order_release);
        sendChangeMessage();
    }
}

void BuiltInDspAudioProcessor::applyVocalProfileEq(AiVocalProfiler::ProfileStyle style)
{
    auto gains = aiVocalProfiler.getGainsForStyle(style);
    setEqEnabled(true);
    setEqLowGainDb(gains.lowGainDb);
    setEqMidGainDb(gains.midGainDb);
    setEqHighGainDb(gains.highGainDb);
    sendChangeMessage();
}
