#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_events/juce_events.h>
#include <juce_core/juce_core.h>
#include <atomic>
#include <memory>

class AudioRecorder : public juce::TimeSliceClient, public juce::ChangeBroadcaster
{
public:
    AudioRecorder();
    ~AudioRecorder() override;

    void prepare(double sampleRate, int samplesPerBlock);

    bool startRecording(juce::String& outErrorMessage);
    void stopRecording();
    bool isRecording() const noexcept { return recordingActive.load(std::memory_order_relaxed); }
    double getRecordingDurationSeconds() const noexcept;

    // Real-time audio ingestion (called from audio processing threads)
    void pushMasterAudio(const float* const* channelData, int numChannels, int numSamples);
    void pushDryMicAudio(const float* const* channelData, int numChannels, int numSamples);

    juce::File getRecordingsFolder() const;
    juce::File getLastMasterRecordingFile() const { return lastMasterFile; }

    int useTimeSlice() override;

private:
    double currentSampleRate{ 44100.0 };
    std::atomic<bool> recordingActive{ false };
    std::atomic<juce::int64> totalSamplesRecorded{ 0 };

    juce::AudioFormatManager formatManager;
    juce::TimeSliceThread backgroundThread{ "LiveStream Recorder Thread" };

    // Ring Buffer for Master (Wet) Audio
    static constexpr int FIFO_CAPACITY = 131072; // ~3 seconds at 44.1kHz
    juce::AbstractFifo masterFifo{ FIFO_CAPACITY };
    juce::AudioBuffer<float> masterFifoBuffer;

    // Ring Buffer for Dry Microphone Audio
    juce::AbstractFifo dryFifo{ FIFO_CAPACITY };
    juce::AudioBuffer<float> dryFifoBuffer;

    // Writers
    std::unique_ptr<juce::AudioFormatWriter> masterWriter;
    std::unique_ptr<juce::AudioFormatWriter> dryWriter;

    juce::File lastMasterFile;
    juce::File lastDryFile;

    juce::CriticalSection writerLock;

    void flushFifos();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioRecorder)
};
