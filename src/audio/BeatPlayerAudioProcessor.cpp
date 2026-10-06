#include "BeatPlayerAudioProcessor.h"

BeatPlayerAudioProcessor::BeatPlayerAudioProcessor()
    : AudioProcessor(BusesProperties().withInput("LiveInput", juce::AudioChannelSet::stereo(), true)
                                      .withOutput("MasterOut", juce::AudioChannelSet::stereo(), true))
{
    formatManager.registerBasicFormats();
}

BeatPlayerAudioProcessor::~BeatPlayerAudioProcessor()
{
    transportSource.setSource(nullptr);
}

void BeatPlayerAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    currentSampleRate = (sampleRate > 8000.0) ? sampleRate : 44100.0;
    transportSource.prepareToPlay(samplesPerBlock, currentSampleRate);
    keyDetector.prepare(currentSampleRate, samplesPerBlock);
    tempBeatBuffer.setSize(2, samplesPerBlock);

    smoothedDuckingGain.reset(currentSampleRate, 0.04); // 40ms smooth ramp
    smoothedDuckingGain.setCurrentAndTargetValue(1.0f);
    duckingHoldSamplesRemaining = 0;
}

void BeatPlayerAudioProcessor::releaseResources()
{
    transportSource.releaseResources();
}

void BeatPlayerAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();
    if (numSamples <= 0 || numChannels <= 0)
        return;

    const auto source = analysisSource.load(std::memory_order_relaxed);

    if (tempBeatBuffer.getNumSamples() < numSamples)
        tempBeatBuffer.setSize(2, numSamples, false, false, true);

    tempBeatBuffer.clear();

    // -------------------------------------------------------------
    // Smart Voice Ducking Analysis (Sidechain from live microphone)
    // -------------------------------------------------------------
    float targetDuck = 1.0f;
    if (duckingEnabled.load(std::memory_order_relaxed))
    {
        float micPeak = 0.0f;
        for (int ch = 0; ch < numChannels; ++ch)
        {
            micPeak = std::max(micPeak, buffer.getMagnitude(ch, 0, numSamples));
        }

        const float threshDb = duckingThresholdDb.load(std::memory_order_relaxed);
        const float micThreshold = juce::Decibels::decibelsToGain(threshDb);
        const float holdMs = duckingHoldMs.load(std::memory_order_relaxed);
        const int holdSamples = static_cast<int>(currentSampleRate * (holdMs * 0.001));

        if (micPeak > micThreshold)
        {
            duckingHoldSamplesRemaining = holdSamples;
            const float reductionDb = duckingAmountDb.load(std::memory_order_relaxed);
            targetDuck = juce::Decibels::decibelsToGain(reductionDb);
        }
        else if (duckingHoldSamplesRemaining > 0)
        {
            duckingHoldSamplesRemaining -= numSamples;
            const float reductionDb = duckingAmountDb.load(std::memory_order_relaxed);
            targetDuck = juce::Decibels::decibelsToGain(reductionDb);
        }
        else
        {
            targetDuck = 1.0f;
        }
    }
    else
    {
        targetDuck = 1.0f;
    }

    smoothedDuckingGain.setTargetValue(targetDuck);
    currentDuckingGain.store(smoothedDuckingGain.getCurrentValue(), std::memory_order_relaxed);

    const bool isCurrentlyPlaying = playing.load(std::memory_order_relaxed);

    if (isCurrentlyPlaying)
    {
        const juce::ScopedLock sl(transportLock);
        
        juce::AudioSourceChannelInfo info(&tempBeatBuffer, 0, numSamples);
        transportSource.getNextAudioBlock(info);

        // Check if track ended and loop is active
        if (transportSource.hasStreamFinished() || (transportSource.getLengthInSeconds() > 0.0 && transportSource.getCurrentPosition() >= transportSource.getLengthInSeconds()))
        {
            if (looping.load(std::memory_order_relaxed))
            {
                transportSource.setPosition(0.0);
                transportSource.start();
            }
            else
            {
                playing.store(false, std::memory_order_release);
            }
        }

        // Apply Beat Volume and Smart Voice Ducking
        const float gain = volumeGain.load(std::memory_order_relaxed);
        tempBeatBuffer.applyGain(gain);
        smoothedDuckingGain.applyGain(tempBeatBuffer, numSamples);

        // Mix Beat into Main Output Buffer
        for (int ch = 0; ch < std::min(numChannels, 2); ++ch)
        {
            buffer.addFrom(ch, 0, tempBeatBuffer, ch, 0, numSamples);
        }
    }

    // Feed chosen audio stream to Key Detector
    if (source == AnalysisSource::BeatPlayer)
    {
        if (isCurrentlyPlaying)
        {
            keyDetector.processBlock(tempBeatBuffer);
        }
    }
    else // LiveMicMaster
    {
        keyDetector.processBlock(buffer);
    }
}

bool BeatPlayerAudioProcessor::loadAudioFile(const juce::File& file, juce::String& errorMsg)
{
    if (!file.existsAsFile())
    {
        errorMsg = "File not found: " + file.getFullPathName();
        return false;
    }

    auto* reader = formatManager.createReaderFor(file);
    if (reader == nullptr)
    {
        errorMsg = "Unsupported audio format. Supported: MP3, WAV, FLAC, OGG, AIFF";
        return false;
    }

    // Offline fast key detection on entire track
    juce::AudioBuffer<float> offlineBuffer(static_cast<int>(reader->numChannels), static_cast<int>(std::min(reader->lengthInSamples, static_cast<juce::int64>(reader->sampleRate * 90.0)))); // Analyze first 90s for instant response
    reader->read(&offlineBuffer, 0, offlineBuffer.getNumSamples(), 0, true, true);
    
    keyDetector.analyzeBufferOffline(offlineBuffer, reader->sampleRate);

    // Setup transport source for playback
    const juce::ScopedLock sl(transportLock);
    transportSource.stop();
    transportSource.setSource(nullptr);
    readerSource.reset();

    readerSource = std::make_unique<juce::AudioFormatReaderSource>(reader, true);
    readerSource->setLooping(looping.load(std::memory_order_relaxed));
    transportSource.setSource(readerSource.get(), 0, nullptr, reader->sampleRate);
    
    currentFileName = file.getFileName();
    playing.store(false, std::memory_order_release);

    sendChangeMessage();
    return true;
}

void BeatPlayerAudioProcessor::play()
{
    const juce::ScopedLock sl(transportLock);
    if (readerSource != nullptr)
    {
        transportSource.start();
        playing.store(true, std::memory_order_release);
        sendChangeMessage();
    }
}

void BeatPlayerAudioProcessor::pause()
{
    const juce::ScopedLock sl(transportLock);
    transportSource.stop();
    playing.store(false, std::memory_order_release);
    sendChangeMessage();
}

void BeatPlayerAudioProcessor::stop()
{
    const juce::ScopedLock sl(transportLock);
    transportSource.stop();
    transportSource.setPosition(0.0);
    playing.store(false, std::memory_order_release);
    sendChangeMessage();
}

void BeatPlayerAudioProcessor::setPosition(double seconds)
{
    const juce::ScopedLock sl(transportLock);
    transportSource.setPosition(seconds);
}

void BeatPlayerAudioProcessor::setLooping(bool shouldLoop)
{
    looping.store(shouldLoop, std::memory_order_release);
    const juce::ScopedLock sl(transportLock);
    if (readerSource != nullptr)
    {
        readerSource->setLooping(shouldLoop);
    }
}

void BeatPlayerAudioProcessor::setGainLinear(float gain)
{
    volumeGain.store(std::clamp(gain, 0.0f, 2.0f), std::memory_order_release);
}

void BeatPlayerAudioProcessor::setAnalysisSource(AnalysisSource src)
{
    analysisSource.store(src, std::memory_order_release);
    keyDetector.reset();
}

double BeatPlayerAudioProcessor::getCurrentPosition() const
{
    return transportSource.getCurrentPosition();
}

double BeatPlayerAudioProcessor::getTotalLength() const
{
    return transportSource.getLengthInSeconds();
}

juce::String BeatPlayerAudioProcessor::getLoadedFileName() const
{
    return currentFileName;
}
