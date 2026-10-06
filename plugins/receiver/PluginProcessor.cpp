#include "PluginProcessor.h"
#include "PluginEditor.h"

OBSReceiverAudioProcessor::OBSReceiverAudioProcessor()
    : AudioProcessor(BusesProperties().withInput("Input", juce::AudioChannelSet::stereo(), true)
                                      .withOutput("Output", juce::AudioChannelSet::stereo(), true))
{
    ipcReceiver.initialize();
}

OBSReceiverAudioProcessor::~OBSReceiverAudioProcessor()
{
    ipcReceiver.close();
}

void OBSReceiverAudioProcessor::prepareToPlay(double sampleRate, int /*samplesPerBlock*/)
{
    currentHostSampleRate = (sampleRate > 1000.0) ? sampleRate : 48000.0;
    ipcReceiver.initialize();
}

void OBSReceiverAudioProcessor::releaseResources()
{
}

void OBSReceiverAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& /*midiMessages*/)
{
    juce::ScopedNoDenormals noDenormals;

    const int totalNumInputChannels = getTotalNumInputChannels();
    const int totalNumOutputChannels = getTotalNumOutputChannels();
    const int numSamples = buffer.getNumSamples();

    // Clear unused output channels
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, numSamples);

    // If IPC is disconnected, try reconnecting
    if (!ipcReceiver.isConnected())
    {
        ipcReceiver.initialize();
    }

    // Pull directly from lock-free shared memory ring buffer with fractional PLL resampling
    float* const* channelPointers = buffer.getArrayOfWritePointers();
    ipcReceiver.readAudio(channelPointers, totalNumOutputChannels, numSamples, currentHostSampleRate);

    // If output is stereo and channel 1 is silent while channel 0 has audio, duplicate to channel 1
    if (totalNumOutputChannels >= 2)
    {
        const float mag0 = buffer.getMagnitude(0, 0, numSamples);
        const float mag1 = buffer.getMagnitude(1, 0, numSamples);
        if (mag0 > 0.000001f && mag1 <= 0.0000001f)
        {
            buffer.copyFrom(1, 0, buffer, 0, 0, numSamples);
        }
    }

    // Calculate RMS for visual feedback in OBS filter panel
    float currentRms = buffer.getRMSLevel(0, 0, numSamples);
    if (totalNumOutputChannels > 1)
        currentRms = std::max(currentRms, buffer.getRMSLevel(1, 0, numSamples));

    rmsLevel.store(currentRms, std::memory_order_relaxed);
}

juce::AudioProcessorEditor* OBSReceiverAudioProcessor::createEditor()
{
    return new OBSReceiverAudioProcessorEditor(*this);
}

void OBSReceiverAudioProcessor::getStateInformation(juce::MemoryBlock& /*destData*/)
{
}

void OBSReceiverAudioProcessor::setStateInformation(const void* /*data*/, int /*sizeInBytes*/)
{
}

// Plugin instantiation entry point
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new OBSReceiverAudioProcessor();
}
