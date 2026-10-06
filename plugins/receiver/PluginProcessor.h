#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "../../src/ipc/SharedMemoryAudioBuffer.h"

class OBSReceiverAudioProcessor : public juce::AudioProcessor
{
public:
    OBSReceiverAudioProcessor();
    ~OBSReceiverAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "LiveStream OBS Receiver"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    bool isConnectedToDaw() const noexcept { return ipcReceiver.isConnected(); }
    float getRmsLevel() const noexcept { return rmsLevel.load(std::memory_order_relaxed); }

private:
    LiveStreamIPC::SharedMemoryAudioReceiver ipcReceiver;
    std::atomic<float> rmsLevel{ 0.0f };
    double currentHostSampleRate{ 48000.0 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OBSReceiverAudioProcessor)
};
