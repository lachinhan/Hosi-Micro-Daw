#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include <atomic>
#include <vector>
#include "TempoSyncEngine.h"
#include "AiNoiseSuppressor.h"

class BuiltInDspAudioProcessor : public juce::AudioProcessor, public juce::ChangeBroadcaster
{
public:
    enum class VocalPreset
    {
        LiveSinging = 0,
        StreamerTalk = 1,
        KaraokeHall = 2,
        PodcastClean = 3,
        BypassAll = 4,
        FactoryReset = 5
    };

    BuiltInDspAudioProcessor();
    ~BuiltInDspAudioProcessor() override = default;

    void resetToFactoryDefaults();

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) override;

    // --- Global Host Tempo (BPM) ---
    void setHostBpm(double bpm) noexcept
    {
        hostBpm.store(bpm, std::memory_order_release);
        if (isReverbBpmSync()) updateReverbParams();
    }
    double getHostBpm() const noexcept { return hostBpm.load(std::memory_order_relaxed); }

    // --- 1. AI Noise & Room De-Reverb Shield ---
    void setAiDenoiseEnabled(bool enabled) noexcept { aiNoiseSuppressor.setEnabled(enabled); sendChangeMessage(); }
    bool isAiDenoiseEnabled() const noexcept { return aiNoiseSuppressor.isEnabled(); }
    void setAiDenoiseAmount(float amount) noexcept { aiNoiseSuppressor.setDenoiseAmount(amount); sendChangeMessage(); }
    float getAiDenoiseAmount() const noexcept { return aiNoiseSuppressor.getDenoiseAmount(); }

    void setAiDeReverbEnabled(bool enabled) noexcept { aiNoiseSuppressor.setDeReverbEnabled(enabled); sendChangeMessage(); }
    bool isAiDeReverbEnabled() const noexcept { return aiNoiseSuppressor.isDeReverbEnabled(); }
    void setAiDeReverbAmount(float amount) noexcept { aiNoiseSuppressor.setDeReverbAmount(amount); sendChangeMessage(); }
    float getAiDeReverbAmount() const noexcept { return aiNoiseSuppressor.getDeReverbAmount(); }

    float getAiNoiseReductionDb() const noexcept { return aiNoiseSuppressor.getNoiseReductionDb(); }
    float getAiVoiceProbability() const noexcept { return aiNoiseSuppressor.getVoiceProbability(); }

    // --- 2. Noise Gate Controls ---
    void setGateEnabled(bool enabled) noexcept
    {
        gateEnabled.store(enabled, std::memory_order_release);
        if (!enabled)
        {
            currentGateGain = 1.0f;
            gateStateOpen = true;
            gateIsOpen.store(true, std::memory_order_release);
        }
    }
    bool isGateEnabled() const noexcept { return gateEnabled.load(std::memory_order_relaxed); }
    void setGateThresholdDb(float threshDb) noexcept { gateThresholdDb.store(threshDb, std::memory_order_release); }
    float getGateThresholdDb() const noexcept { return gateThresholdDb.load(std::memory_order_relaxed); }
    bool isGateOpen() const noexcept { return gateIsOpen.load(std::memory_order_relaxed); }

    // --- 3-Band Studio EQ Controls ---
    void setEqEnabled(bool enabled) noexcept { eqEnabled.store(enabled, std::memory_order_release); }
    bool isEqEnabled() const noexcept { return eqEnabled.load(std::memory_order_relaxed); }
    void setEqLowGainDb(float gainDb) noexcept { eqLowGainDb.store(gainDb, std::memory_order_release); needEqUpdate.store(true, std::memory_order_release); }
    float getEqLowGainDb() const noexcept { return eqLowGainDb.load(std::memory_order_relaxed); }
    void setEqMidGainDb(float gainDb) noexcept { eqMidGainDb.store(gainDb, std::memory_order_release); needEqUpdate.store(true, std::memory_order_release); }
    float getEqMidGainDb() const noexcept { return eqMidGainDb.load(std::memory_order_relaxed); }
    void setEqHighGainDb(float gainDb) noexcept { eqHighGainDb.store(gainDb, std::memory_order_release); needEqUpdate.store(true, std::memory_order_release); }
    float getEqHighGainDb() const noexcept { return eqHighGainDb.load(std::memory_order_relaxed); }

    // --- Warm Compressor Controls ---
    void setCompEnabled(bool enabled) noexcept
    {
        compEnabled.store(enabled, std::memory_order_release);
        if (!enabled)
        {
            currentCompGain = 1.0f;
            compGainReductionDb.store(0.0f, std::memory_order_release);
        }
    }
    bool isCompEnabled() const noexcept { return compEnabled.load(std::memory_order_relaxed); }
    void setCompThresholdDb(float threshDb) noexcept { compThresholdDb.store(threshDb, std::memory_order_release); }
    float getCompThresholdDb() const noexcept { return compThresholdDb.load(std::memory_order_relaxed); }
    void setCompRatio(float ratio) noexcept { compRatio.store(ratio, std::memory_order_release); }
    float getCompRatio() const noexcept { return compRatio.load(std::memory_order_relaxed); }
    void setCompMakeupDb(float makeupDb) noexcept { compMakeupDb.store(makeupDb, std::memory_order_release); }
    float getCompMakeupDb() const noexcept { return compMakeupDb.load(std::memory_order_relaxed); }
    float getCompGainReductionDb() const noexcept { return compGainReductionDb.load(std::memory_order_relaxed); }

    // --- Lush Reverb Controls & Smart BPM Auto-Tail ---
    void setReverbEnabled(bool enabled) noexcept { reverbEnabled.store(enabled, std::memory_order_release); }
    bool isReverbEnabled() const noexcept { return reverbEnabled.load(std::memory_order_relaxed); }
    void setReverbSize(float size) noexcept { reverbSize.store(size, std::memory_order_release); updateReverbParams(); }
    float getReverbSize() const noexcept { return reverbSize.load(std::memory_order_relaxed); }
    void setReverbDamp(float damp) noexcept { reverbDamp.store(damp, std::memory_order_release); updateReverbParams(); }
    float getReverbDamp() const noexcept { return reverbDamp.load(std::memory_order_relaxed); }
    void setReverbWetMix(float wet) noexcept { reverbWetMix.store(wet, std::memory_order_release); }
    float getReverbWetMix() const noexcept { return reverbWetMix.load(std::memory_order_relaxed); }

    void setReverbBpmSync(bool sync) noexcept { reverbBpmSync.store(sync, std::memory_order_release); updateReverbParams(); }
    bool isReverbBpmSync() const noexcept { return reverbBpmSync.load(std::memory_order_relaxed); }
    void setReverbBarLength(TempoSyncEngine::ReverbBarLength bars) noexcept { reverbBarLength.store(bars, std::memory_order_release); updateReverbParams(); }
    TempoSyncEngine::ReverbBarLength getReverbBarLength() const noexcept { return reverbBarLength.load(std::memory_order_relaxed); }

    // --- Stereo Delay / Echo Controls & Smart BPM Subdivision ---
    void setDelayEnabled(bool enabled) noexcept { delayEnabled.store(enabled, std::memory_order_release); }
    bool isDelayEnabled() const noexcept { return delayEnabled.load(std::memory_order_relaxed); }
    void setDelayTimeMs(float timeMs) noexcept { delayTimeMs.store(timeMs, std::memory_order_release); }
    float getDelayTimeMs() const noexcept { return delayTimeMs.load(std::memory_order_relaxed); }
    void setDelayFeedback(float fb) noexcept { delayFeedback.store(fb, std::memory_order_release); }
    float getDelayFeedback() const noexcept { return delayFeedback.load(std::memory_order_relaxed); }
    void setDelayWetMix(float wet) noexcept { delayWetMix.store(wet, std::memory_order_release); }
    float getDelayWetMix() const noexcept { return delayWetMix.load(std::memory_order_relaxed); }

    void setDelayBpmSync(bool sync) noexcept { delayBpmSync.store(sync, std::memory_order_release); }
    bool isDelayBpmSync() const noexcept { return delayBpmSync.load(std::memory_order_relaxed); }
    void setDelaySubdivision(TempoSyncEngine::DelaySubdivision div) noexcept { delaySubdivision.store(div, std::memory_order_release); }
    TempoSyncEngine::DelaySubdivision getDelaySubdivision() const noexcept { return delaySubdivision.load(std::memory_order_relaxed); }

    float getEffectiveDelayTimeMs() const noexcept
    {
        if (isDelayBpmSync())
        {
            return TempoSyncEngine::calculateDelayTimeMs(getHostBpm(), getDelaySubdivision());
        }
        return getDelayTimeMs();
    }

    // --- Brickwall Limiter Controls ---
    void setLimiterEnabled(bool enabled) noexcept { limiterEnabled.store(enabled, std::memory_order_release); }
    bool isLimiterEnabled() const noexcept { return limiterEnabled.load(std::memory_order_relaxed); }
    void setLimiterThresholdDb(float threshDb) noexcept { limiterThresholdDb.store(threshDb, std::memory_order_release); }
    float getLimiterThresholdDb() const noexcept { return limiterThresholdDb.load(std::memory_order_relaxed); }

    // Preset selection
    void loadPreset(VocalPreset preset);
    VocalPreset getCurrentPreset() const noexcept { return currentPreset.load(std::memory_order_relaxed); }

    // Standard AudioProcessor boilerplate
    const juce::String getName() const override { return "Built-In Studio DSP Vocal Suite"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 2.0; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}
    void getStateInformation(juce::MemoryBlock&) override;
    void setStateInformation(const void*, int) override;
    juce::AudioProcessorEditor* createEditor() override { return nullptr; }
    bool hasEditor() const override { return false; }

private:
    double currentSampleRate{ 48000.0 };
    std::atomic<double> hostBpm{ 120.0 };

    // --- Noise Gate State ---
    std::atomic<bool> gateEnabled{ false };
    std::atomic<float> gateThresholdDb{ -48.0f };
    std::atomic<bool> gateIsOpen{ true };
    float gateEnvelope{ 0.0f };
    float currentGateGain{ 1.0f };
    bool gateStateOpen{ true };
    int gateHoldSamplesRemaining{ 0 };

    // --- EQ Filters ---
    std::atomic<bool> eqEnabled{ false };
    std::atomic<float> eqLowGainDb{ 0.0f };
    std::atomic<float> eqMidGainDb{ 2.0f };
    std::atomic<float> eqHighGainDb{ 2.5f };
    std::atomic<bool> needEqUpdate{ true };

    juce::dsp::IIR::Filter<float> lowShelfL, lowShelfR;
    juce::dsp::IIR::Filter<float> midPeakL, midPeakR;
    juce::dsp::IIR::Filter<float> highShelfL, highShelfR;

    void updateEqCoefficients();

    // --- Compressor State ---
    std::atomic<bool> compEnabled{ false };
    std::atomic<float> compThresholdDb{ -18.0f };
    std::atomic<float> compRatio{ 3.2f };
    std::atomic<float> compMakeupDb{ 2.5f };
    std::atomic<float> compGainReductionDb{ 0.0f };
    float compEnvelope{ 0.0f };
    float currentCompGain{ 1.0f };

    // --- Reverb ---
    std::atomic<bool> reverbEnabled{ true };
    std::atomic<float> reverbSize{ 0.65f };
    std::atomic<float> reverbDamp{ 0.35f };
    std::atomic<float> reverbWetMix{ 0.22f };
    std::atomic<bool> reverbBpmSync{ true };
    std::atomic<TempoSyncEngine::ReverbBarLength> reverbBarLength{ TempoSyncEngine::ReverbBarLength::OneBar };

    juce::Reverb reverbProcessor;
    juce::Reverb::Parameters reverbParams;
    juce::AudioBuffer<float> tempReverbBuffer;
    void updateReverbParams();

    // --- Delay ---
    std::atomic<bool> delayEnabled{ false };
    std::atomic<float> delayTimeMs{ 260.0f };
    std::atomic<float> delayFeedback{ 0.25f };
    std::atomic<float> delayWetMix{ 0.18f };
    std::atomic<bool> delayBpmSync{ true };
    std::atomic<TempoSyncEngine::DelaySubdivision> delaySubdivision{ TempoSyncEngine::DelaySubdivision::DottedEighth };

    juce::AudioBuffer<float> delayBuffer;
    int delayWritePos{ 0 };
    float delayLowPassL{ 0.0f };
    float delayLowPassR{ 0.0f };
    float currentSmoothedDelaySamplesL{ 0.0f };
    float currentSmoothedDelaySamplesR{ 0.0f };

    // --- Limiter ---
    std::atomic<bool> limiterEnabled{ false };
    std::atomic<float> limiterThresholdDb{ -0.5f };
    float limiterPeakEnv{ 0.0f };

    std::atomic<VocalPreset> currentPreset{ VocalPreset::BypassAll };

    // --- AI Noise Suppressor & Room De-Reverb ---
    AiNoiseSuppressor aiNoiseSuppressor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BuiltInDspAudioProcessor)
};
