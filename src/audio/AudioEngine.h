#pragma once

#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include "GraphManager.h"
#include "../ipc/SharedMemoryAudioBuffer.h"

class AudioEngine : public juce::ChangeListener
{
public:
    AudioEngine();
    ~AudioEngine() override;

    void initialize();
    void shutdown();

    void saveDeviceState();
    void restoreDeviceState();

    juce::AudioDeviceManager& getDeviceManager() noexcept { return deviceManager; }
    GraphManager& getGraphManager() noexcept { return graphManager; }

    LiveStreamIPC::AudioApiMode getCurrentApiMode() const noexcept { return currentApiMode.load(std::memory_order_relaxed); }
    juce::String getCurrentDeviceTypeName() const;

    void changeListenerCallback(juce::ChangeBroadcaster* source) override;

    double getSampleRate() const;
    int getBufferSize() const;
    float getCpuUsage() const;

private:
    juce::AudioDeviceManager deviceManager;
    juce::AudioProcessorPlayer processorPlayer;
    GraphManager graphManager;

    std::atomic<LiveStreamIPC::AudioApiMode> currentApiMode{ LiveStreamIPC::AudioApiMode::WasapiShared };

    void updateCurrentApiMode();
    juce::File getDeviceSettingsFile() const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioEngine)
};
