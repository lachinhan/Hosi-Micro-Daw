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

    // Pre-allocate temp reverb buffer to eliminate heap allocation on audio thread
    tempReverbBuffer.setSize(2, std::max(samplesPerBlock, 2048));

    // Noise Gate Smoother
    gateGainSmooth.reset(currentSampleRate, 0.005); // 5ms ramp
    gateGainSmooth.setCurrentAndTargetValue(1.0f);
    gateEnvelope = 0.0f;

    // Compressor Smoother
    compGainSmooth.reset(currentSampleRate, 0.005);
    compGainSmooth.setCurrentAndTargetValue(1.0f);
    compEnvelope = 0.0f;

    // EQ Filters Reset
    lowShelfL.reset(); lowShelfR.reset();
    midPeakL.reset(); midPeakR.reset();
    highShelfL.reset(); highShelfR.reset();
    needEqUpdate.store(true, std::memory_order_release);
    updateEqCoefficients();

    // Reverb Reset
    reverbProcessor.setSampleRate(currentSampleRate);
    updateReverbParams();

    // Delay Buffer (Max 2.0 seconds)
    const int maxDelaySamples = static_cast<int>(currentSampleRate * 2.0) + 1024;
    delayBuffer.setSize(2, maxDelaySamples);
    delayBuffer.clear();
    delayWritePos = 0;
    delayLowPassL = 0.0f;
    delayLowPassR = 0.0f;

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
    reverbParams.roomSize = std::clamp(reverbSize.load(std::memory_order_relaxed), 0.0f, 1.0f);
    reverbParams.damping = std::clamp(reverbDamp.load(std::memory_order_relaxed), 0.0f, 1.0f);
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

    const bool hasGate = gateEnabled.load(std::memory_order_relaxed);
    const bool hasEq = eqEnabled.load(std::memory_order_relaxed);
    const bool hasComp = compEnabled.load(std::memory_order_relaxed);
    const bool hasDelay = delayEnabled.load(std::memory_order_relaxed);
    const bool hasReverb = reverbEnabled.load(std::memory_order_relaxed);
    const bool hasLimiter = limiterEnabled.load(std::memory_order_relaxed);

    // If completely bypassed, return immediately with 0 overhead
    if (!hasGate && !hasEq && !hasComp && !hasDelay && !hasReverb && !hasLimiter)
        return;

    // Check EQ coefficient updates
    if (hasEq && needEqUpdate.load(std::memory_order_relaxed))
    {
        updateEqCoefficients();
    }

    float* left = buffer.getWritePointer(0);
    float* right = (numChannels > 1) ? buffer.getWritePointer(1) : left;

    // -------------------------------------------------------------
    // 1. NOISE GATE (Fast Attack, Smooth Release & Hysteresis)
    // -------------------------------------------------------------
    if (hasGate)
    {
        const float threshDb = gateThresholdDb.load(std::memory_order_relaxed);
        const float threshLin = juce::Decibels::decibelsToGain(threshDb);
        const float closeThreshLin = threshLin * 0.707f; // Hysteresis (-3 dB lower to close)

        const float gateAttackAlpha = static_cast<float>(std::exp(-1.0 / (currentSampleRate * 0.002)));
        const float gateReleaseAlpha = static_cast<float>(std::exp(-1.0 / (currentSampleRate * 0.120)));

        for (int i = 0; i < numSamples; ++i)
        {
            const float inputPeak = std::max(std::abs(left[i]), std::abs(right[i]));

            if (inputPeak > gateEnvelope)
                gateEnvelope = gateAttackAlpha * gateEnvelope + (1.0f - gateAttackAlpha) * inputPeak;
            else
                gateEnvelope = gateReleaseAlpha * gateEnvelope + (1.0f - gateReleaseAlpha) * inputPeak;

            bool isOpen = (gateEnvelope > closeThreshLin);
            if (gateEnvelope > threshLin)
                isOpen = true;

            gateIsOpen.store(isOpen, std::memory_order_relaxed);

            float targetGain = isOpen ? 1.0f : 0.0f;
            gateGainSmooth.setTargetValue(targetGain);

            const float g = gateGainSmooth.getNextValue();
            left[i] *= g;
            right[i] *= g;
        }
    }
    else
    {
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
            compGainSmooth.setTargetValue(targetCompGain);

            const float cg = compGainSmooth.getNextValue();
            left[i] *= cg;
            right[i] *= cg;

            // Subtle warm harmonic saturation (soft-knee analog vibe)
            left[i] = std::tanh(left[i] * 0.95f);
            right[i] = std::tanh(right[i] * 0.95f);
        }

        compGainReductionDb.store(maxReductionDb, std::memory_order_relaxed);
    }
    else
    {
        compGainReductionDb.store(0.0f, std::memory_order_relaxed);
    }

    // -------------------------------------------------------------
    // 4. STEREO DELAY / ECHO (Ping-Pong + Tape Lowpass Warmth)
    // -------------------------------------------------------------
    if (delayEnabled.load(std::memory_order_relaxed) && delayBuffer.getNumSamples() > 0)
    {
        const float wet = delayWetMix.load(std::memory_order_relaxed);
        const float fb = std::clamp(delayFeedback.load(std::memory_order_relaxed), 0.0f, 0.85f);
        const float timeMs = std::clamp(delayTimeMs.load(std::memory_order_relaxed), 20.0f, 1500.0f);

        const int delayOffsetL = std::clamp(static_cast<int>(currentSampleRate * (timeMs * 0.001f)), 1, delayBuffer.getNumSamples() - 1);
        const int delayOffsetR = std::clamp(static_cast<int>(currentSampleRate * (timeMs * 0.00135f)), 1, delayBuffer.getNumSamples() - 1); // Slight stereo offset for 3D width

        const int bufferSize = delayBuffer.getNumSamples();
        float* dL = delayBuffer.getWritePointer(0);
        float* dR = delayBuffer.getWritePointer(1);

        const float lpfAlpha = 0.28f; // ~4.5kHz analog tape high cut

        for (int i = 0; i < numSamples; ++i)
        {
            const float inL = left[i];
            const float inR = right[i];

            int readPosL = (delayWritePos - delayOffsetL + bufferSize) % bufferSize;
            int readPosR = (delayWritePos - delayOffsetR + bufferSize) % bufferSize;

            float delayedL = dL[readPosL];
            float delayedR = dR[readPosR];

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
    // 5. LUSH STUDIO REVERB (Stereo Plate / Chamber Space)
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

    switch (preset)
    {
    case VocalPreset::LiveSinging:
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
        setReverbSize(0.70f);
        setReverbDamp(0.35f);
        setReverbWetMix(0.26f);
        setDelayEnabled(true);
        setDelayTimeMs(280.0f);
        setDelayFeedback(0.28f);
        setDelayWetMix(0.18f);
        setLimiterEnabled(true);
        setLimiterThresholdDb(-0.5f);
        break;

    case VocalPreset::StreamerTalk:
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
        setReverbSize(0.40f);
        setReverbDamp(0.50f);
        setReverbWetMix(0.08f);
        setDelayEnabled(false);
        setLimiterEnabled(true);
        setLimiterThresholdDb(-0.5f);
        break;

    case VocalPreset::KaraokeHall:
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
        setReverbSize(0.82f);
        setReverbDamp(0.25f);
        setReverbWetMix(0.35f);
        setDelayEnabled(true);
        setDelayTimeMs(320.0f);
        setDelayFeedback(0.35f);
        setDelayWetMix(0.24f);
        setLimiterEnabled(true);
        setLimiterThresholdDb(-0.5f);
        break;

    case VocalPreset::PodcastClean:
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
    xml.setAttribute("delayEnabled", isDelayEnabled());
    xml.setAttribute("delayTime", getDelayTimeMs());
    xml.setAttribute("delayFeedback", getDelayFeedback());
    xml.setAttribute("delayWet", getDelayWetMix());
    xml.setAttribute("limiterEnabled", isLimiterEnabled());
    xml.setAttribute("preset", static_cast<int>(getCurrentPreset()));

    copyXmlToBinary(xml, destData);
}

void BuiltInDspAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    auto xml = getXmlFromBinary(data, sizeInBytes);
    if (xml != nullptr && xml->hasTagName("BuiltInDspState"))
    {
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
        setDelayEnabled(xml->getBoolAttribute("delayEnabled", false));
        setDelayTimeMs(static_cast<float>(xml->getDoubleAttribute("delayTime", 260.0)));
        setDelayFeedback(static_cast<float>(xml->getDoubleAttribute("delayFeedback", 0.25)));
        setDelayWetMix(static_cast<float>(xml->getDoubleAttribute("delayWet", 0.18)));
        setLimiterEnabled(xml->getBoolAttribute("limiterEnabled", true));
        currentPreset.store(static_cast<VocalPreset>(xml->getIntAttribute("preset", 0)), std::memory_order_release);
        sendChangeMessage();
    }
}
