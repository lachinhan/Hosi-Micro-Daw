#include "AudioEngine.h"

AudioEngine::AudioEngine()
{
}

AudioEngine::~AudioEngine()
{
    shutdown();
}

juce::File AudioEngine::getDeviceSettingsFile() const
{
    auto appDir = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
                     .getChildFile("LiveStreamMicroDAW");
    if (!appDir.exists())
        appDir.createDirectory();
    return appDir.getChildFile("audio_settings.xml");
}

void AudioEngine::initialize()
{
    // Try restoring previously saved audio device configuration (ASIO, device, buffer size, channels)
    auto settingsFile = getDeviceSettingsFile();
    std::unique_ptr<juce::XmlElement> savedXml;
    if (settingsFile.existsAsFile())
    {
        savedXml = juce::XmlDocument::parse(settingsFile);
    }

    if (savedXml != nullptr)
    {
        deviceManager.initialise(2, 8, savedXml.get(), true);
    }
    else
    {
        // First run default low-latency setup
        juce::AudioDeviceManager::AudioDeviceSetup setup;
        deviceManager.getAudioDeviceSetup(setup);
        setup.sampleRate = 48000.0;
        setup.bufferSize = 128; // Ultra-low latency 128 samples

        deviceManager.initialise(2, 8, nullptr, true, {}, &setup);
    }

    deviceManager.addChangeListener(this);

    // Initialise audio graph
    graphManager.initializeGraph();

    // Link processor player with graph and device callback
    processorPlayer.setProcessor(&graphManager.getGraph());
    deviceManager.addAudioCallback(&processorPlayer);

    updateCurrentApiMode();
}

void AudioEngine::saveDeviceState()
{
    auto xml = deviceManager.createStateXml();
    if (xml != nullptr)
    {
        xml->writeTo(getDeviceSettingsFile());
    }
}

void AudioEngine::shutdown()
{
    saveDeviceState();

    deviceManager.removeChangeListener(this);
    deviceManager.removeAudioCallback(&processorPlayer);
    processorPlayer.setProcessor(nullptr);
    deviceManager.closeAudioDevice();
}

void AudioEngine::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    if (source == &deviceManager)
    {
        updateCurrentApiMode();
        saveDeviceState(); // Auto-save when user changes device or buffer size in settings
    }
}

void AudioEngine::updateCurrentApiMode()
{
    juce::AudioIODevice* currentDevice = deviceManager.getCurrentAudioDevice();
    if (currentDevice != nullptr)
    {
        const juce::String typeName = currentDevice->getTypeName();
        if (typeName.containsIgnoreCase("ASIO"))
        {
            currentApiMode.store(LiveStreamIPC::AudioApiMode::AsioExclusive, std::memory_order_release);
            graphManager.setApiMode(LiveStreamIPC::AudioApiMode::AsioExclusive);
        }
        else
        {
            currentApiMode.store(LiveStreamIPC::AudioApiMode::WasapiShared, std::memory_order_release);
            graphManager.setApiMode(LiveStreamIPC::AudioApiMode::WasapiShared);
        }
    }
}

juce::String AudioEngine::getCurrentDeviceTypeName() const
{
    juce::AudioIODevice* currentDevice = const_cast<juce::AudioDeviceManager&>(deviceManager).getCurrentAudioDevice();
    if (currentDevice != nullptr)
        return currentDevice->getTypeName();
    return "No Device";
}

double AudioEngine::getSampleRate() const
{
    juce::AudioIODevice* currentDevice = const_cast<juce::AudioDeviceManager&>(deviceManager).getCurrentAudioDevice();
    if (currentDevice != nullptr)
        return currentDevice->getCurrentSampleRate();
    return 48000.0;
}

int AudioEngine::getBufferSize() const
{
    juce::AudioIODevice* currentDevice = const_cast<juce::AudioDeviceManager&>(deviceManager).getCurrentAudioDevice();
    if (currentDevice != nullptr)
        return currentDevice->getCurrentBufferSizeSamples();
    return 128;
}

float AudioEngine::getCpuUsage() const
{
    return static_cast<float>(deviceManager.getCpuUsage() * 100.0);
}
