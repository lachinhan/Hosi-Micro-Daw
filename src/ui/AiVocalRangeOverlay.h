#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_gui_extra/juce_gui_extra.h>
#include "../audio/VocalRangeDetector.h"
#include "../songbook/SongbookManager.h"
#include "../audio/KeyDetector.h"

class AiVocalRangeOverlay : public juce::Component, private juce::Timer
{
public:
    AiVocalRangeOverlay(VocalRangeDetector& detector, SongbookManager& songbookMgr);
    ~AiVocalRangeOverlay() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void visibilityChanged() override;

    std::function<void()> onCloseClicked;
    std::function<void(int rootNote, KeyDetector::ScaleType scale, const juce::String& songName)> onApplyTone;
    std::function<void(double bpm, const juce::String& songName)> onApplyTempo;
    std::function<void(const juce::String& songName)> onPlayYouTubeBeat;
    std::function<void()> onOpenFullSongbookAi;

private:
    void timerCallback() override;

    VocalRangeDetector& rangeDetector;
    SongbookManager& songbookManager;

    // Header Controls
    juce::Label titleLabel;
    juce::Label subtitleLabel;
    juce::TextButton closeButton{ "✕" };

    // Range Bar Visualizer Component
    class RangeBarVisualizer : public juce::Component
    {
    public:
        RangeBarVisualizer(VocalRangeDetector& det) : detector(det) {}

        void paint(juce::Graphics& g) override;

    private:
        VocalRangeDetector& detector;
    };

    RangeBarVisualizer rangeBarVisualizer;

    // Scan Controls
    juce::TextButton startScanButton{ juce::String::fromUTF8(u8"🎙️ BẮT ĐẦU ĐO ÂM VỰC (5s QUÉT GIỌNG)") };
    juce::Label scanStatusLabel;
    juce::ProgressBar scanProgressBar;
    double progressValue{ 0.0 };

    // Manual Adjust Controls
    juce::Label manualLabel;
    juce::Label lowNoteLabel{ {}, juce::String::fromUTF8(u8"Nốt Trầm Nhất:") };
    juce::ComboBox lowNoteCombo;
    juce::Label highNoteLabel{ {}, juce::String::fromUTF8(u8"Nốt Cao Nhất:") };
    juce::ComboBox highNoteCombo;

    // Vocal Classification Banner
    juce::Label vocalClassBanner;

    // Top Recommended Songs List
    juce::Label recommendSectionLabel;
    
    struct SongRow
    {
        std::unique_ptr<juce::Label> titleLabel;
        std::unique_ptr<juce::Label> infoLabel;
        std::unique_ptr<juce::Label> badgeLabel;
        std::unique_ptr<juce::TextButton> singButton;
    };
    std::vector<SongRow> songRows;

    juce::TextButton viewAllInSongbookBtn{ juce::String::fromUTF8(u8"🎯 XEM TOÀN BỘ BÀI PHÙ HỢP TRONG SỔ TONE") };
    juce::TextButton saveAndCloseBtn{ juce::String::fromUTF8(u8"💾 LƯU & HOÀN TẤT") };

    void setupNoteCombos();
    void updateRecommendations();
    void applySongRecommendation(const SongItem& song, const SongbookManager::SongFitResult& fit);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AiVocalRangeOverlay)
};
