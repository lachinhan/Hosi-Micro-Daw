#pragma once

#include <juce_gui_extra/juce_gui_extra.h>
#include "../audio/KeyDetector.h"
#include "../songbook/SongbookManager.h"

class YouTubePlayerOverlay : public juce::Component, public juce::TextEditor::Listener
{
public:
    YouTubePlayerOverlay(SongbookManager* songbookMgr = nullptr);
    ~YouTubePlayerOverlay() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void textEditorReturnKeyPressed(juce::TextEditor& editor) override;
    bool keyPressed(const juce::KeyPress& key) override;

    void searchAndPlay(const juce::String& songName);
    void loadUrl(const juce::String& url);

    void selectMaleTone(bool announce = true);
    void selectFemaleTone(bool announce = true);
    void toggleDuetTone();

    std::function<void()> onCloseClicked;
    std::function<void()> onDetectAndPushToAutoTune;
    std::function<void(int rootNote, KeyDetector::ScaleType scale, const juce::String& sourceName, bool showNotificationPopup)> onApplyTone;
    std::function<void()> onOpenSongbook;

private:
    SongbookManager* songbookManager{ nullptr };
    juce::Label titleLabel;
    juce::TextButton closeButton{ "X" };

    // Base Tone Tracking for Auto-Tune Transposition Sync
    int currentBaseRootNote{ 9 }; // Default Am (A = 9)
    KeyDetector::ScaleType currentBaseScale{ KeyDetector::ScaleType::Minor };
    bool hasActiveBaseTone{ false };
    juce::String activeSongName{ "" };

    // Duet Male / Female Tone Tracking
    juce::String maleTone{ "Am" };
    juce::String femaleTone{ "Dm" };
    enum class ActiveDuetGender { None, Male, Female };
    ActiveDuetGender activeGender{ ActiveDuetGender::None };

    // Tone & Auto-Tune Direct Controls
    juce::TextButton detectKeyButton{ juce::String::fromUTF8(u8"🎯 DÒ TONE (AUTO-KEY)") };
    juce::TextButton manualToneButton{ juce::String::fromUTF8(u8"⚡ NẠP AUTO-TUNE") };
    juce::TextButton songbookQuickButton{ juce::String::fromUTF8(u8"🎵 SỔ TONE") };

    // Song Ca / Duet Switcher Buttons
    juce::TextButton maleToneButton{ juce::String::fromUTF8(u8"♂ NAM") };
    juce::TextButton femaleToneButton{ juce::String::fromUTF8(u8"♀ NỮ") };

    // Search Box
    juce::TextEditor searchEditor;
    juce::TextButton searchButton{ juce::String::fromUTF8(u8"🔍 TÌM BEAT") };

    // Quick Action & Tone Helper Buttons
    juce::TextButton cleanModeButton{ juce::String::fromUTF8(u8"🛡️ CHẶN QC / LIVE") };
    juce::TextButton refreshButton{ juce::String::fromUTF8(u8"🔄 TẢI LẠI") };

    // Pitch Shifter Helper Buttons
    juce::Label pitchLabel{ {}, juce::String::fromUTF8(u8"TONE:") };
    juce::TextButton pitchDown3Btn{ "-3" };
    juce::TextButton pitchDown2Btn{ "-2" };
    juce::TextButton pitchDown1Btn{ "-1" };
    juce::TextButton pitchResetBtn{ "0" };
    juce::TextButton pitchUp1Btn{ "+1" };
    juce::TextButton pitchUp2Btn{ "+2" };
    juce::TextButton pitchUp3Btn{ "+3" };
    int currentPitchShift{ 0 };
    juce::String currentUrl{ "https://www.youtube.com/results?search_query=karaoke+viet+nam+beat+chuan" };

    // Native Web View Browser Component
    std::unique_ptr<juce::WebBrowserComponent> webBrowser;

    void updateDuetButtonsUI();
    void executePitchShift(int semitones);
    void injectAdSkipScript();
    void showManualToneMenu();
    void detectKeyFromYouTubeTitleOrAudio();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(YouTubePlayerOverlay)
};
