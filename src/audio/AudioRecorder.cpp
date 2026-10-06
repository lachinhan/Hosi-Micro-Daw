#include "AudioRecorder.h"

AudioRecorder::AudioRecorder()
{
    formatManager.registerBasicFormats();
    masterFifoBuffer.setSize(2, FIFO_CAPACITY);
    dryFifoBuffer.setSize(2, FIFO_CAPACITY);
    backgroundThread.startThread(juce::Thread::Priority::normal);
}

AudioRecorder::~AudioRecorder()
{
    stopRecording();
    backgroundThread.stopThread(2000);
}

void AudioRecorder::prepare(double sampleRate, int /*samplesPerBlock*/)
{
    currentSampleRate = (sampleRate > 8000.0) ? sampleRate : 44100.0;
}

juce::File AudioRecorder::getRecordingsFolder() const
{
    // Try application directory "recordings/" first
    auto exeFile = juce::File::getSpecialLocation(juce::File::currentExecutableFile);
    auto appDir = exeFile.getParentDirectory();
    auto recDir = appDir.getChildFile("recordings");

    if (!recDir.exists())
        recDir.createDirectory();

    return recDir;
}

bool AudioRecorder::startRecording(juce::String& outErrorMessage)
{
    if (recordingActive.load(std::memory_order_relaxed))
    {
        return true;
    }

    auto recDir = getRecordingsFolder();
    if (!recDir.isDirectory())
    {
        outErrorMessage = "Cannot create or access recordings directory: " + recDir.getFullPathName();
        return false;
    }

    auto timeStamp = juce::Time::getCurrentTime().formatted("%Y-%m-%d_%H-%M-%S");
    lastMasterFile = recDir.getChildFile("Master_Mix_" + timeStamp + ".wav");
    lastDryFile = recDir.getChildFile("Mic_Dry_" + timeStamp + ".wav");

    juce::WavAudioFormat wavFormat;

    // 1. Create Master Output Writer (24-bit Stereo)
    auto masterOutStream = lastMasterFile.createOutputStream();
    if (masterOutStream == nullptr)
    {
        outErrorMessage = "Failed to create master output file stream.";
        return false;
    }

    auto* mWriter = wavFormat.createWriterFor(masterOutStream.get(), currentSampleRate, 2, 24, {}, 0);
    if (mWriter == nullptr)
    {
        outErrorMessage = "Failed to create WAV writer for master mix.";
        return false;
    }
    masterOutStream.release(); // AudioFormatWriter takes ownership

    // 2. Create Dry Mic Writer (24-bit Stereo)
    auto dryOutStream = lastDryFile.createOutputStream();
    if (dryOutStream == nullptr)
    {
        delete mWriter;
        outErrorMessage = "Failed to create dry mic file stream.";
        return false;
    }

    auto* dWriter = wavFormat.createWriterFor(dryOutStream.get(), currentSampleRate, 2, 24, {}, 0);
    if (dWriter == nullptr)
    {
        delete mWriter;
        outErrorMessage = "Failed to create WAV writer for dry mic.";
        return false;
    }
    dryOutStream.release();

    {
        const juce::ScopedLock sl(writerLock);
        masterWriter.reset(mWriter);
        dryWriter.reset(dWriter);
    }

    // Reset FIFOs
    masterFifo.reset();
    dryFifo.reset();
    totalSamplesRecorded.store(0, std::memory_order_release);
    recordingActive.store(true, std::memory_order_release);

    backgroundThread.addTimeSliceClient(this);
    sendChangeMessage();
    return true;
}

void AudioRecorder::stopRecording()
{
    if (!recordingActive.load(std::memory_order_relaxed))
        return;

    recordingActive.store(false, std::memory_order_release);
    backgroundThread.removeTimeSliceClient(this);

    // Flush any remaining samples to disk
    flushFifos();

    {
        const juce::ScopedLock sl(writerLock);
        masterWriter.reset();
        dryWriter.reset();
    }

    sendChangeMessage();
}

double AudioRecorder::getRecordingDurationSeconds() const noexcept
{
    if (currentSampleRate <= 0.0) return 0.0;
    return static_cast<double>(totalSamplesRecorded.load(std::memory_order_relaxed)) / currentSampleRate;
}

void AudioRecorder::pushMasterAudio(const float* const* channelData, int numChannels, int numSamples)
{
    if (!recordingActive.load(std::memory_order_relaxed) || numSamples <= 0 || channelData == nullptr)
        return;

    int start1, size1, start2, size2;
    masterFifo.prepareToWrite(numSamples, start1, size1, start2, size2);

    if (size1 > 0)
    {
        for (int ch = 0; ch < std::min(2, numChannels); ++ch)
            masterFifoBuffer.copyFrom(ch, start1, channelData[ch], size1);
        if (numChannels == 1)
            masterFifoBuffer.copyFrom(1, start1, channelData[0], size1);
    }

    if (size2 > 0)
    {
        for (int ch = 0; ch < std::min(2, numChannels); ++ch)
            masterFifoBuffer.copyFrom(ch, start2, channelData[ch] + size1, size2);
        if (numChannels == 1)
            masterFifoBuffer.copyFrom(1, start2, channelData[0] + size1, size2);
    }

    masterFifo.finishedWrite(size1 + size2);
    totalSamplesRecorded.fetch_add(numSamples, std::memory_order_relaxed);
}

void AudioRecorder::pushDryMicAudio(const float* const* channelData, int numChannels, int numSamples)
{
    if (!recordingActive.load(std::memory_order_relaxed) || numSamples <= 0 || channelData == nullptr)
        return;

    int start1, size1, start2, size2;
    dryFifo.prepareToWrite(numSamples, start1, size1, start2, size2);

    if (size1 > 0)
    {
        for (int ch = 0; ch < std::min(2, numChannels); ++ch)
            dryFifoBuffer.copyFrom(ch, start1, channelData[ch], size1);
        if (numChannels == 1)
            dryFifoBuffer.copyFrom(1, start1, channelData[0], size1);
    }

    if (size2 > 0)
    {
        for (int ch = 0; ch < std::min(2, numChannels); ++ch)
            dryFifoBuffer.copyFrom(ch, start2, channelData[ch] + size1, size2);
        if (numChannels == 1)
            dryFifoBuffer.copyFrom(1, start2, channelData[0] + size1, size2);
    }

    dryFifo.finishedWrite(size1 + size2);
}

int AudioRecorder::useTimeSlice()
{
    flushFifos();
    return 10; // Call again in 10ms
}

void AudioRecorder::flushFifos()
{
    const juce::ScopedLock sl(writerLock);

    // 1. Flush Master Mix
    if (masterWriter != nullptr)
    {
        int numReady = masterFifo.getNumReady();
        while (numReady > 0)
        {
            int start1, size1, start2, size2;
            masterFifo.prepareToRead(numReady, start1, size1, start2, size2);

            if (size1 > 0)
            {
                masterWriter->writeFromAudioSampleBuffer(masterFifoBuffer, start1, size1);
            }
            if (size2 > 0)
            {
                masterWriter->writeFromAudioSampleBuffer(masterFifoBuffer, start2, size2);
            }

            masterFifo.finishedRead(size1 + size2);
            numReady = masterFifo.getNumReady();
        }
    }

    // 2. Flush Dry Mic
    if (dryWriter != nullptr)
    {
        int numReady = dryFifo.getNumReady();
        while (numReady > 0)
        {
            int start1, size1, start2, size2;
            dryFifo.prepareToRead(numReady, start1, size1, start2, size2);

            if (size1 > 0)
            {
                dryWriter->writeFromAudioSampleBuffer(dryFifoBuffer, start1, size1);
            }
            if (size2 > 0)
            {
                dryWriter->writeFromAudioSampleBuffer(dryFifoBuffer, start2, size2);
            }

            dryFifo.finishedRead(size1 + size2);
            numReady = dryFifo.getNumReady();
        }
    }
}
