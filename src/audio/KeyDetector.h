#pragma once

#include <juce_core/juce_core.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>
#include <array>
#include <atomic>
#include <cmath>
#include <vector>

/**
 * High-precision Musical Key & Scale Detector (Chromagram + Krumhansl-Schmuckler algorithm).
 * Analyzes audio in real-time or from audio buffers to determine the musical Root Note and Scale (Major/Minor).
 */
class KeyDetector
{
public:
    enum class ScaleType
    {
        Major = 0,
        Minor = 1,
        Unknown = 2
    };

    struct KeyResult
    {
        int rootNote{ 0 };             // 0 = C, 1 = C#, 2 = D, ..., 11 = B
        ScaleType scale{ ScaleType::Unknown };
        float confidence{ 0.0f };       // 0.0 to 1.0
        bool isLocked{ false };         // True when key convergence is reached or offline scan completes
        juce::String keyName{ "--" };   // e.g. "C Major", "F# Minor"
        std::array<float, 12> chromaProfile{}; // Raw 12-pitch class energy (0.0 to 1.0)
    };

    KeyDetector();
    ~KeyDetector() = default;

    void prepare(double sampleRate, int samplesPerBlock);
    void reset();
    void unlock();

    // Process real-time streaming audio chunk (mono or stereo)
    void processBlock(const juce::AudioBuffer<float>& buffer);

    // Analyze an entire audio buffer (e.g. loaded backing track) in one fast offline pass
    KeyResult analyzeBufferOffline(const juce::AudioBuffer<float>& buffer, double sampleRate);

    // Get current thread-safe real-time detection result
    KeyResult getCurrentResult() const;

    // Static helper to get musical note name
    static juce::String getNoteName(int noteIndex);
    static juce::String formatKeyName(int rootNote, ScaleType scale);

private:
    static constexpr int FFT_ORDER = 12; // 4096-point FFT for crisp low-frequency pitch resolution
    static constexpr int FFT_SIZE = 1 << FFT_ORDER; // 4096 samples
    static constexpr int HOP_SIZE = 2048; // 50% overlap

    double currentSampleRate{ 44100.0 };
    juce::dsp::FFT fft{ FFT_ORDER };
    juce::dsp::WindowingFunction<float> window{ FFT_SIZE, juce::dsp::WindowingFunction<float>::hann };

    // Circular input buffer for FFT framing
    std::vector<float> fifo;
    int fifoIndex{ 0 };
    std::array<float, FFT_SIZE * 2> fftData{};

    // Accumulated chroma profile with long-term memory
    std::array<float, 12> accumulatedChroma{};
    std::array<float, 12> instantChromaSmooth{};
    int framesProcessed{ 0 };

    // Key Convergence & Locking tracker
    int candidateRoot{ -1 };
    ScaleType candidateScale{ ScaleType::Unknown };
    int consecutiveStableFrames{ 0 };
    bool keyLocked{ false };

    // Thread-safe cached result
    mutable juce::CriticalSection resultLock;
    KeyResult latestResult;

    // Pitch frequency lookup table
    std::array<float, FFT_SIZE / 2> binToMidiNote{};
    std::array<int, FFT_SIZE / 2> binToPitchClass{};
    std::array<float, FFT_SIZE / 2> binWeight{};

    void initializePitchLookup(double sampleRate);
    void processFFTFrame();
    KeyResult calculateKeyFromChroma(const std::array<float, 12>& chroma) const;
};
