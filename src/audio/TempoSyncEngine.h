#pragma once

#include <juce_core/juce_core.h>
#include <juce_events/juce_events.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>
#include <atomic>
#include <vector>
#include <array>
#include <chrono>

/**
 * Smart Studio BPM & Tempo Synchronization Engine.
 * Provides high-precision tempo tracking, Tap-Tempo averaging, 
 * subdivision delay math, bar-synchronized reverb decay calculations,
 * and real-time transient onset beat detection.
 */
class TempoSyncEngine : public juce::ChangeBroadcaster
{
public:
    enum class DelaySubdivision
    {
        Quarter = 0,       // 1/4 Note (1.0x) - Standard pop delay
        DottedEighth = 1,  // 1/8 Dotted (0.75x) - Classic Ballad / Modern Pop bounce
        Eighth = 2,        // 1/8 Note (0.5x) - Energetic sync
        TripletEighth = 3, // 1/8 Triplet (0.3333x) - Fast triplet bounce
        Sixteenth = 4,     // 1/16 Note (0.25x) - Rapid slapback
        Half = 5,          // 1/2 Note (2.0x) - Ambient space
        Free = 6           // Manual millisecond control
    };

    enum class ReverbBarLength
    {
        HalfBar = 0,  // 1/2 Bar - Short & Crisp (Fast Rap / EDM)
        OneBar = 1,   // 1 Bar (⭐ Studio Standard) - Reverb decays cleanly at bar end
        TwoBars = 2,  // 2 Bars - Warm & Lush (Ballad / Bolero)
        FourBars = 3, // 4 Bars - Deep Ambient / Cathedral
        Free = 4      // Manual Size & Damp sliders
    };

    TempoSyncEngine();
    ~TempoSyncEngine() override = default;

    // --- Global BPM Management ---
    void setBpm(double newBpm, const juce::String& source = "Manual");
    double getBpm() const noexcept { return currentBpm.load(std::memory_order_relaxed); }
    juce::String getBpmSource() const;

    // --- Tap Tempo ---
    void tapTempo();
    void resetTapHistory();

    // --- Musical Math Helpers ---
    static float calculateDelayTimeMs(double bpm, DelaySubdivision subdivision);
    static float calculateReverbDecaySec(double bpm, ReverbBarLength barLength);
    static juce::String getSubdivisionName(DelaySubdivision subdivision);
    static juce::String getBarLengthName(ReverbBarLength barLength);

    // --- Real-Time Audio Beat / Onset Transient Detector ---
    void prepare(double sampleRate, int samplesPerBlock);
    void resetDetector();
    void processAudioBlock(const juce::AudioBuffer<float>& buffer);

    struct BeatDetectionResult
    {
        double estimatedBpm{ 120.0 };
        float confidence{ 0.0f }; // 0.0 to 1.0
        bool isStable{ false };
    };

    BeatDetectionResult getDetectedBeatResult() const;

private:
    std::atomic<double> currentBpm{ 120.0 };
    mutable juce::CriticalSection stateLock;
    juce::String lastBpmSource{ "Default" };

    // Tap Tempo state
    std::vector<std::chrono::steady_clock::time_point> tapHistory;
    static constexpr int MAX_TAP_HISTORY = 5;
    static constexpr int64_t TAP_TIMEOUT_MS = 2500; // Reset after 2.5s inactivity

    // Real-Time Beat / Onset Transient Estimation
    double currentSampleRate{ 44100.0 };
    static constexpr int ONSET_HOP_SIZE = 512;
    static constexpr int ONSET_BUFFER_SIZE = 2048; // ~46ms analysis frames
    static constexpr int TEMPO_HISTOGRAM_BINS = 180; // 60 to 240 BPM

    std::vector<float> inputFifo;
    int fifoWritePos{ 0 };
    float previousFrameEnergy{ 0.0f };

    // Ring buffer of spectral energy flux onsets (~3 seconds at ~86 fps)
    static constexpr int ONSET_RING_SIZE = 256;
    std::array<float, ONSET_RING_SIZE> onsetFluxRing{};
    int onsetRingPos{ 0 };
    int onsetFrameCounter{ 0 };

    BeatDetectionResult latestBeatResult;
    mutable juce::CriticalSection beatLock;

    void processOnsetFrame(const float* channelData, int numSamples);
    void computeAutocorrelationTempo();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TempoSyncEngine)
};
