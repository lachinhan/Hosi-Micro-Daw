#pragma once

#include <juce_core/juce_core.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>
#include <atomic>
#include <vector>
#include <array>
#include <cmath>

/**
 * AI Vocal Profiler & Smart Auto-EQ Analyzer (1-Click AI Sounding).
 * 
 * Captures 5 seconds of dry vocal input, extracts spectral balance, formant distribution,
 * room resonance (mud), sibilance harshness, and high-frequency air.
 * Then generates ideal compensation EQ curves for Studio, Bolero, Pop/Remix, and Podcast.
 */
class AiVocalProfiler
{
public:
    enum class VocalType
    {
        Unknown,
        MaleBaritone,  // Nam Trầm / Dày
        MaleTenor,     // Nam Cao / Sáng
        FemaleAlto,    // Nữ Trung / Ấm
        FemaleSoprano  // Nữ Cao / Thanh thoát
    };

    enum class ProfileStyle
    {
        StudioMaster = 0, // Cân bằng chuẩn Studio (Tự nhiên, trong trẻo)
        SweetBolero,      // Bolero & Ballad (Ấm áp, dày giọng, ngọt ngào)
        RemixPop,         // Nhạc Trẻ / Remix / Pop (Sáng bay bổng, lực, cắt đục)
        PodcastStreamer   // Streamer / Podcast (Rõ chữ, trong vắt, khử ù phòng)
    };

    struct ProfileResult
    {
        bool isValid{ false };
        VocalType vocalType{ VocalType::Unknown };
        juce::String vocalTypeName;
        
        // Diagnostic metrics
        float fundamentalHz{ 0.0f };
        float mudEnergyRatio{ 0.0f };     // 200 - 400 Hz room boom ratio
        float sibilanceRatio{ 0.0f };     // 5k - 8.5 kHz harshness ratio
        float airRatio{ 0.0f };           // 9k - 18 kHz air presence ratio
        
        juce::String diagnosticSummary;
        juce::String recommendation;

        // Recommended EQ gains per style (Low, Mid, High in dB)
        struct EqGains
        {
            float lowGainDb{ 0.0f };
            float midGainDb{ 0.0f };
            float highGainDb{ 0.0f };
        };

        EqGains styles[4];
    };

    AiVocalProfiler();
    ~AiVocalProfiler() = default;

    void prepare(double sampleRate, int samplesPerBlock);
    void reset();

    // Start 5-second recording analysis
    void startProfiling();
    void cancelProfiling();

    bool isProfiling() const noexcept { return isRecording.load(std::memory_order_relaxed); }
    float getProgress() const noexcept { return recordingProgress.load(std::memory_order_relaxed); }

    // Feeds audio stream during profiling
    void processBlock(const juce::AudioBuffer<float>& buffer);

    // Results
    const ProfileResult& getLastResult() const noexcept { return lastResult; }
    ProfileResult::EqGains getGainsForStyle(ProfileStyle style) const;

private:
    std::atomic<bool> isRecording{ false };
    std::atomic<float> recordingProgress{ 0.0f };

    double currentSampleRate{ 44100.0 };
    int totalRecordedSamples{ 0 };
    int maxRecordSamples{ 44100 * 5 }; // 5 seconds default

    std::vector<float> recordBuffer;
    ProfileResult lastResult;

    void analyzeRecordedAudio();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AiVocalProfiler)
};
