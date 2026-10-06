#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include "../ipc/SharedMemoryAudioBuffer.h"

class IpcEmitterAudioProcessor : public juce::AudioProcessor
{
public:
    IpcEmitterAudioProcessor();
    ~IpcEmitterAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) override;

    juce::AudioProcessorEditor* createEditor() override { return nullptr; }
    bool hasEditor() const override { return false; }

    const juce::String getName() const override { return "Master Out & IPC Emitter"; }
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

    void setEnabled(bool shouldBeEnabled) noexcept { isEmitterEnabled.store(shouldBeEnabled, std::memory_order_release); }
    bool isEnabled() const noexcept { return isEmitterEnabled.load(std::memory_order_relaxed); }

    void setApiMode(LiveStreamIPC::AudioApiMode mode) noexcept;

    // Master Volume Gain (0.0 to 2.0 linear, where 1.0 = 0dB)
    void setMasterGain(float linearGain) noexcept;
    float getMasterGain() const noexcept { return targetGain.load(std::memory_order_relaxed); }

    // Real-time stereo metering
    float getLeftPeak() const noexcept { return leftPeak.load(std::memory_order_relaxed); }
    float getRightPeak() const noexcept { return rightPeak.load(std::memory_order_relaxed); }

    void setAudioRecorder(class AudioRecorder* rec) noexcept { recorder = rec; }

private:
    class AudioRecorder* recorder{ nullptr };
    LiveStreamIPC::SharedMemoryAudioSender ipcSender;
    std::atomic<bool> isEmitterEnabled{ true };
    std::atomic<LiveStreamIPC::AudioApiMode> currentApiMode{ LiveStreamIPC::AudioApiMode::WasapiShared };

    std::atomic<float> targetGain{ 1.0f };
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> smoothedGain;

    std::atomic<float> leftPeak{ 0.0f };
    std::atomic<float> rightPeak{ 0.0f };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(IpcEmitterAudioProcessor)
};
