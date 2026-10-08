#pragma once

#include <juce_core/juce_core.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <atomic>
#include <vector>
#include <array>
#include <cmath>

/**
 * AI Vocal Range Detector & Pitch Tracking Engine.
 * 
 * Analyzes real-time microphone voice input to extract fundamental frequency (F0),
 * tracks the user's lowest and highest comfortable pitch limits, and classifies vocal type
 * (Baritone, Tenor, Alto, Soprano).
 */
class VocalRangeDetector
{
public:
    enum class VocalClass
    {
        Unknown = 0,
        MaleBaritone, // Nam Trầm (E2 - E4)
        MaleTenor,    // Nam Cao (A2 - A4 / C5)
        FemaleAlto,   // Nữ Trung (D3 - D5)
        FemaleSoprano // Nữ Cao (G3 - C6)
    };

    struct RangeProfile
    {
        int lowestMidi{ 48 };       // Default C3 (MIDI 48)
        int highestMidi{ 69 };      // Default A4 (MIDI 69)
        VocalClass vocalClass{ VocalClass::MaleTenor };
        juce::String vocalClassName{ juce::String::fromUTF8(u8"Nam Cao (Tenor)") };
        bool isCalibrated{ false };

        int getSpanSemitones() const noexcept { return std::max(0, highestMidi - lowestMidi); }
        juce::String getLowestNoteName() const { return midiToNoteName(lowestMidi); }
        juce::String getHighestNoteName() const { return midiToNoteName(highestMidi); }
    };

    struct RealtimePitch
    {
        float currentHz{ 0.0f };
        int currentMidi{ 0 };
        float confidence{ 0.0f }; // 0.0 to 1.0
        bool isVoiceActive{ false };
        juce::String noteName{ "--" };
    };

    VocalRangeDetector();
    ~VocalRangeDetector() = default;

    void prepare(double sampleRate, int samplesPerBlock);
    void reset();

    // Calibration / 5-second Vocal Range Scan
    void startScan(float durationSeconds = 6.0f);
    void stopScan();
    bool isScanning() const noexcept { return isScanningActive.load(std::memory_order_relaxed); }
    float getScanProgress() const noexcept { return scanProgress.load(std::memory_order_relaxed); }

    // Live continuous pitch tracking mode
    void setLiveTrackingEnabled(bool enabled) noexcept { liveTrackingEnabled.store(enabled, std::memory_order_release); }
    bool isLiveTrackingEnabled() const noexcept { return liveTrackingEnabled.load(std::memory_order_relaxed); }

    // Audio stream ingestion
    void processBlock(const juce::AudioBuffer<float>& buffer);

    // Profile getters & manual setters
    const RangeProfile& getProfile() const noexcept { return profile; }
    void setCustomRange(int lowestMidi, int highestMidi);
    
    // Thread-safe real-time state for UI
    RealtimePitch getLivePitch() const;

    // Load / Save user vocal profile from disk
    void loadProfileFromDisk();
    void saveProfileToDisk();

    // Static musical utilities
    static juce::String midiToNoteName(int midiNote);
    static int noteNameToMidi(const juce::String& noteName);
    static float midiToFrequency(int midiNote) noexcept;
    static float frequencyToMidi(float hz) noexcept;
    static juce::String classifyVocalTypeName(int lowestMidi, int highestMidi, VocalClass& outClass);

    std::function<void(const RangeProfile&)> onProfileUpdated;

private:
    double currentSampleRate{ 44100.0 };
    std::atomic<bool> isScanningActive{ false };
    std::atomic<float> scanProgress{ 0.0f };
    std::atomic<bool> liveTrackingEnabled{ true };

    int scanTotalSamplesTarget{ 44100 * 6 };
    int scanRecordedSamples{ 0 };
    int tempScanLowestMidi{ 127 };
    int tempScanHighestMidi{ 0 };
    int validPitchesSampled{ 0 };

    RangeProfile profile;

    // Realtime Pitch Lock-free state
    std::atomic<float> atomicLiveHz{ 0.0f };
    std::atomic<int> atomicLiveMidi{ 0 };
    std::atomic<float> atomicLiveConfidence{ 0.0f };
    std::atomic<bool> atomicLiveActive{ false };

    // Circular input buffer for pitch tracking
    static constexpr int ANALYSIS_WINDOW = 2048;
    std::vector<float> ringBuffer;
    int ringWritePos{ 0 };
    int samplesSinceLastDetect{ 0 };

    // Pitch smoothing filter
    int consecutivePitchMatches{ 0 };
    int lastDetectedMidi{ 0 };

    void detectPitchFromWindow();
    void updateClassification();

    juce::File getProfileFile() const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VocalRangeDetector)
};
