#include "IpcEmitterAudioProcessor.h"
#include "AudioRecorder.h"

IpcEmitterAudioProcessor::IpcEmitterAudioProcessor()
    : AudioProcessor(BusesProperties().withInput("Input", juce::AudioChannelSet::stereo(), true)
                                      .withOutput("Output", juce::AudioChannelSet::stereo(), true))
{
    ipcSender.initialize();
}

IpcEmitterAudioProcessor::~IpcEmitterAudioProcessor()
{
    ipcSender.close();
}

void IpcEmitterAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    ipcSender.initialize();
    ipcSender.setSampleRate(static_cast<uint32_t>(sampleRate));

    smoothedGain.reset(sampleRate, 0.05); // 50ms smoothing for pop-free volume change
    smoothedGain.setCurrentAndTargetValue(targetGain.load(std::memory_order_relaxed));

    if (recorder != nullptr)
        recorder->prepare(sampleRate, samplesPerBlock);
}

void IpcEmitterAudioProcessor::releaseResources()
{
}

void IpcEmitterAudioProcessor::setApiMode(LiveStreamIPC::AudioApiMode mode) noexcept
{
    currentApiMode.store(mode, std::memory_order_release);
    ipcSender.setAudioApiMode(mode);
}

void IpcEmitterAudioProcessor::setMasterGain(float linearGain) noexcept
{
    targetGain.store(linearGain, std::memory_order_release);
}

void IpcEmitterAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& /*midiMessages*/)
{
    juce::ScopedNoDenormals noDenormals;

    const int numChannels = buffer.getNumChannels();
    const int numSamples = buffer.getNumSamples();

    if (numSamples <= 0 || numChannels <= 0)
        return;

    // Apply master smoothed gain
    const float newTarget = targetGain.load(std::memory_order_relaxed);
    smoothedGain.setTargetValue(newTarget);
    smoothedGain.applyGain(buffer, numSamples);

    // If channel 1 is completely silent while channel 0 has signal, duplicate ch 0 -> ch 1
    if (numChannels >= 2)
    {
        const float mag0 = buffer.getMagnitude(0, 0, numSamples);
        const float mag1 = buffer.getMagnitude(1, 0, numSamples);
        if (mag0 > 0.000001f && mag1 <= 0.0000001f)
        {
            buffer.copyFrom(1, 0, buffer, 0, 0, numSamples);
        }
    }

    // Compute peak levels for stereo meters
    float lPeak = buffer.getMagnitude(0, 0, numSamples);
    float rPeak = (numChannels > 1) ? buffer.getMagnitude(1, 0, numSamples) : lPeak;

    // Fast attack, smooth decay
    float prevL = leftPeak.load(std::memory_order_relaxed);
    float prevR = rightPeak.load(std::memory_order_relaxed);
    leftPeak.store(std::max(lPeak, prevL * 0.85f), std::memory_order_relaxed);
    rightPeak.store(std::max(rPeak, prevR * 0.85f), std::memory_order_relaxed);

    // Stream to OBS Shared Memory if enabled (Supports both ASIO and WASAPI modes)
    if (isEmitterEnabled.load(std::memory_order_relaxed))
    {
        const float* const* channelPointers = buffer.getArrayOfReadPointers();
        ipcSender.writeAudio(channelPointers, numChannels, numSamples);
    }

    // Push Master (Wet) Audio to Recorder
    if (recorder != nullptr && recorder->isRecording())
    {
        recorder->pushMasterAudio(buffer.getArrayOfReadPointers(), numChannels, numSamples);
    }
}
