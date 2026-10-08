#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_audio_devices/juce_audio_devices.h>
#include "KeyDetector.h"
#include "TempoSyncEngine.h"

class BeatPlayerAudioProcessor : public juce::AudioProcessor, public juce::ChangeBroadcaster
{
public:
    enum class AnalysisSource
    {
        BeatPlayer = 0,
        LiveMicMaster = 1
    };

    BeatPlayerAudioProcessor();
    ~BeatPlayerAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) override;

    // Tempo Sync Engine Connection
    void setTempoSyncEngine(TempoSyncEngine* engine) noexcept { tempoSyncEngine = engine; }
    TempoSyncEngine* getTempoSyncEngine() noexcept { return tempoSyncEngine; }

    // File playback controls
    bool loadAudioFile(const juce::File& file, juce::String& errorMsg);
    void play();
    void pause();
    void stop();
    void setPosition(double seconds);
    void setLooping(bool shouldLoop);
    void setGainLinear(float gain);
    void setAnalysisSource(AnalysisSource src);

    // Smart Voice Ducking (Radio Talk-over)
    void setDuckingEnabled(bool enabled) noexcept { duckingEnabled.store(enabled, std::memory_order_release); }
    bool isDuckingEnabled() const noexcept { return duckingEnabled.load(std::memory_order_relaxed); }
    void setDuckingAmountDb(float duckDb) noexcept { duckingAmountDb.store(duckDb, std::memory_order_release); }
    float getDuckingAmountDb() const noexcept { return duckingAmountDb.load(std::memory_order_relaxed); }
    void setDuckingThresholdDb(float threshDb) noexcept { duckingThresholdDb.store(threshDb, std::memory_order_release); }
    float getDuckingThresholdDb() const noexcept { return duckingThresholdDb.load(std::memory_order_relaxed); }
    void setDuckingHoldMs(float holdMs) noexcept { duckingHoldMs.store(holdMs, std::memory_order_release); }
    float getDuckingHoldMs() const noexcept { return duckingHoldMs.load(std::memory_order_relaxed); }
    float getCurrentDuckingGain() const noexcept { return currentDuckingGain.load(std::memory_order_relaxed); }

    bool isPlaying() const noexcept { return playing.load(std::memory_order_relaxed); }
    bool isLooping() const noexcept { return looping.load(std::memory_order_relaxed); }
    double getCurrentPosition() const;
    double getTotalLength() const;
    float getGainLinear() const noexcept { return volumeGain.load(std::memory_order_relaxed); }
    juce::String getLoadedFileName() const;
    AnalysisSource getAnalysisSource() const noexcept { return analysisSource.load(std::memory_order_relaxed); }

    KeyDetector& getKeyDetector() noexcept { return keyDetector; }
    KeyDetector::KeyResult getDetectedKey() const { return keyDetector.getCurrentResult(); }

    // Standard AudioProcessor boilerplate
    const juce::String getName() const override { return "Beat Track & Auto Key Detector"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}
    void getStateInformation(juce::MemoryBlock&) override {}
    void setStateInformation(const void*, int) override {}
    juce::AudioProcessorEditor* createEditor() override { return nullptr; }
    bool hasEditor() const override { return false; }

private:
    juce::AudioFormatManager formatManager;
    std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
    juce::AudioTransportSource transportSource;

    KeyDetector keyDetector;
    TempoSyncEngine* tempoSyncEngine{ nullptr };

    std::atomic<AnalysisSource> analysisSource{ AnalysisSource::BeatPlayer };
    std::atomic<bool> playing{ false };
    std::atomic<bool> looping{ true };
    std::atomic<float> volumeGain{ 1.0f };
    std::atomic<bool> duckingEnabled{ false };
    std::atomic<float> duckingAmountDb{ -12.0f };
    std::atomic<float> duckingThresholdDb{ -36.0f };
    std::atomic<float> duckingHoldMs{ 500.0f };
    std::atomic<float> currentDuckingGain{ 1.0f };
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> smoothedDuckingGain;
    int duckingHoldSamplesRemaining{ 0 };

    juce::String currentFileName{ "No Beat Loaded" };
    double currentSampleRate{ 44100.0 };

    juce::AudioBuffer<float> tempBeatBuffer;
    juce::CriticalSection transportLock;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BeatPlayerAudioProcessor)
};
