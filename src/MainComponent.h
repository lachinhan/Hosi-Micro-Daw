#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "audio/AudioEngine.h"
#include "presets/PresetManager.h"
#include "ui/VerticalRackComponent.h"
#include "ui/MasterChannelStripComponent.h"
#include "ui/AudioSettingsOverlay.h"
#include "ui/DonateOverlay.h"
#include "ui/SongbookOverlay.h"
#include "ui/AiVocalRangeOverlay.h"
#include "ui/KeyDetectorComponent.h"
#include "ui/SoundboardComponent.h"
#include "ui/BuiltInDspComponent.h"
#include "songbook/SongbookManager.h"
#include "utils/UpdateChecker.h"
#if HOSI_PRO_EDITION
#include "ui/YouTubePlayerOverlay.h"
#endif

class CompactModeIconButton : public juce::Button
{
public:
    CompactModeIconButton() : juce::Button("CompactToggle") {}

    void setMiniMode(bool isMini)
    {
        miniMode = isMini;
        repaint();
    }

    void paintButton(juce::Graphics& g, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
        auto bounds = getLocalBounds().toFloat().reduced(1.0f);
        
        juce::Colour bgCol = miniMode ? juce::Colour(0xff0284c7) : juce::Colour(0xff1e293b);
        if (shouldDrawButtonAsDown)
            bgCol = bgCol.darker(0.25f);
        else if (shouldDrawButtonAsHighlighted)
            bgCol = bgCol.brighter(0.18f);

        g.setColour(bgCol);
        g.fillRoundedRectangle(bounds, 4.0f);

        g.setColour(miniMode ? juce::Colour(0xff38bdf8) : juce::Colour(0xff334155));
        g.drawRoundedRectangle(bounds, 4.0f, 1.0f);

        // Crisp Vector Arrow
        juce::Path p;
        const float cx = bounds.getCentreX();
        const float cy = bounds.getCentreY();
        const float halfW = 5.0f;
        const float halfH = 3.5f;

        if (miniMode)
        {
            // Mini mode active -> arrow points DOWN (expand back to full rack)
            p.startNewSubPath(cx - halfW, cy - halfH);
            p.lineTo(cx + halfW, cy - halfH);
            p.lineTo(cx, cy + halfH);
            p.closeSubPath();
        }
        else
        {
            // Full mode active -> arrow points UP (collapse to mini bar)
            p.startNewSubPath(cx - halfW, cy + halfH);
            p.lineTo(cx + halfW, cy + halfH);
            p.lineTo(cx, cy - halfH);
            p.closeSubPath();
        }

        g.setColour(miniMode ? juce::Colours::white : juce::Colour(0xff38bdf8));
        g.fillPath(p);
    }

private:
    bool miniMode{ false };
};

class MainComponent : public juce::Component, public juce::Timer
{
public:
    MainComponent();
    ~MainComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void timerCallback() override;
    bool keyPressed(const juce::KeyPress& key) override;

private:
    AudioEngine audioEngine;
    PresetManager presetManager;

    // Header UI elements
    juce::Label titleLabel;
    juce::Label modeBadgeLabel;
    juce::Label statsLabel;

    // Mic Master Mute button
    juce::TextButton muteMicButton{ "MIC ON" };

    // Input Channel Source Selector
    juce::TextButton inputSourceButton{ "IN: MIC 1 (L+R)" };

    // Quick Scene / Preset buttons (F1, F2, F3)
    juce::TextButton liveSingingPresetBtn{ "LIVE (F1)" };
    juce::TextButton talkPresetBtn{ "TALK (F2)" };
    juce::TextButton trapPresetBtn{ "TUNE (F3)" };

    // Custom Preset File Management
    juce::TextButton savePresetBtn{ "SAVE" };
    juce::TextButton loadPresetBtn{ "LOAD" };

    // Streamer Floating Mini-Bar / Compact Mode Vector Button
    CompactModeIconButton compactModeButton;
    juce::TextButton alwaysOnTopButton{ "PIN" };

    juce::TextButton settingsButton{ "SETTINGS" };
    juce::TextButton updateCheckButton{ "UPDATE" };
    juce::TextButton donateButton{ juce::String::fromUTF8(u8"☕ DONATE") };

    // Beat Player & Auto Key Detector Bar
    std::unique_ptr<KeyDetectorComponent> keyDetectorBar;

    // Rack container
    std::unique_ptr<VerticalRackComponent> verticalRack;

    // Right Column: Tab Switch Buttons (Master Strip vs Built-In DSP vs Soundboard)
    enum class RightTab
    {
        Master = 0,
        VocalDsp = 1,
        Soundboard = 2
    };

    juce::TextButton masterTabButton{ "MASTER" };
    juce::TextButton vocalDspTabButton{ "VOCAL DSP" };
    juce::TextButton soundboardTabButton{ "SOUND FX" };
    RightTab currentRightTab{ RightTab::Master };

    // Master Output Channel Strip (Fader + Dual Stereo Meter)
    std::unique_ptr<MasterChannelStripComponent> masterStrip;

    // Built-in Studio Vocal DSP Panel
    std::unique_ptr<BuiltInDspComponent> builtInDspPanel;

    // Soundboard Panel
    std::unique_ptr<SoundboardComponent> soundboardPanel;

    // Settings overlay
    std::unique_ptr<AudioSettingsOverlay> settingsOverlay;
    bool isSettingsOverlayVisible{ false };

    // Donate overlay
    std::unique_ptr<class DonateOverlay> donateOverlay;
    bool isDonateOverlayVisible{ false };

    // Songbook & Tone Manager
    SongbookManager songbookManager;
    juce::TextButton songbookButton{ juce::String::fromUTF8(u8"🎵 SỔ TONE") };
    std::unique_ptr<SongbookOverlay> songbookOverlay;
    bool isSongbookOverlayVisible{ false };

    // AI Vocal Range & Song Recommendation Overlay
    std::unique_ptr<class AiVocalRangeOverlay> vocalRangeOverlay;
    bool isVocalRangeOverlayVisible{ false };

#if HOSI_PRO_EDITION
    // Mini YouTube Karaoke Player (PRO Feature)
    juce::TextButton youtubeButton{ juce::String::fromUTF8(u8"📺 YOUTUBE BEAT") };
    std::unique_ptr<YouTubePlayerOverlay> youtubeOverlay;
    bool isYouTubeOverlayVisible{ false };
#endif

    bool isCompactMode{ false };
    bool wasInCompactModeBeforeOverlay{ false };
    bool isAlwaysOnTop{ false };
    int previousFullWidth{ 1140 };
    int previousFullHeight{ 760 };

    void setRightTab(RightTab tab);
    void toggleCompactMode();
    void toggleAlwaysOnTop();
    void applyUiScale(float scaleFactor, int targetW = 1140, int targetH = 760);

    void showSettings(bool show);
    void showDonate(bool show);
    void showSongbook(bool show);
    void showVocalRangeOverlay(bool show);
#if HOSI_PRO_EDITION
    void showYouTubePlayer(bool show, const juce::String& initialSongName = {});
#endif

    void updatePresetButtonsUI();
    void updateMuteButtonUI();
    void updateInputSourceButtonUI();
    void updateRightTabButtonsUI();
    void handleSavePreset();
    void handleLoadPreset();

    float smoothedCpuUsage{ 0.0f };
    int timerTicks{ 0 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
