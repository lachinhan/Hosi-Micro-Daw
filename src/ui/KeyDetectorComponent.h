#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../audio/BeatPlayerAudioProcessor.h"
#include "../audio/GraphManager.h"
#include "../audio/TempoSyncEngine.h"

enum class TransportIconType
{
    PlayPause,
    Stop,
    Loop
};

class TransportIconButton : public juce::Button
{
public:
    TransportIconButton(TransportIconType type, const juce::String& name)
        : juce::Button(name), iconType(type) {}

    void setPlaying(bool playing) { isPlaying = playing; repaint(); }
    void setLoopActive(bool active) { isLoopActive = active; repaint(); }

    void paintButton(juce::Graphics& g, bool isMouseOverButton, bool isButtonDown) override
    {
        auto bounds = getLocalBounds().toFloat().reduced(1.5f);
        
        // Background Pill Card
        juce::Colour bgCol(0xff1e293b);
        if (iconType == TransportIconType::PlayPause)
        {
            bgCol = isPlaying ? juce::Colour(0xffd97706) : juce::Colour(0xff059669);
        }
        else if (iconType == TransportIconType::Loop && isLoopActive)
        {
            bgCol = juce::Colour(0xff0284c7);
        }

        if (isButtonDown)
            bgCol = bgCol.darker(0.25f);
        else if (isMouseOverButton)
            bgCol = bgCol.brighter(0.20f);

        g.setColour(bgCol);
        g.fillRoundedRectangle(bounds, 5.0f);

        g.setColour(bgCol.brighter(0.35f));
        g.drawRoundedRectangle(bounds, 5.0f, 1.0f);

        // Render Vector Icon in center
        auto center = bounds.getCentre();
        g.setColour(juce::Colours::white);

        if (iconType == TransportIconType::PlayPause)
        {
            if (isPlaying)
            {
                // Pause Bars: two vertical rounded rectangles
                const float barW = 3.5f;
                const float barH = 11.0f;
                const float gap = 2.5f;
                g.fillRoundedRectangle(center.x - gap - barW, center.y - barH * 0.5f, barW, barH, 1.0f);
                g.fillRoundedRectangle(center.x + gap, center.y - barH * 0.5f, barW, barH, 1.0f);
            }
            else
            {
                // Play Triangle
                juce::Path p;
                const float size = 10.0f;
                p.addTriangle(center.x - size * 0.35f, center.y - size * 0.55f,
                              center.x - size * 0.35f, center.y + size * 0.55f,
                              center.x + size * 0.55f, center.y);
                g.fillPath(p);
            }
        }
        else if (iconType == TransportIconType::Stop)
        {
            // Stop Square
            const float sqSize = 9.0f;
            g.fillRoundedRectangle(center.x - sqSize * 0.5f, center.y - sqSize * 0.5f, sqSize, sqSize, 1.5f);
        }
        else if (iconType == TransportIconType::Loop)
        {
            // Loop Icon: circular repeat arrow
            juce::Path p;
            const float r = 5.0f;
            p.addCentredArc(center.x, center.y, r, r, 0.0f, 0.6f, juce::MathConstants<float>::twoPi - 0.7f, true);
            g.strokePath(p, juce::PathStrokeType(1.6f));

            // Arrow head
            juce::Path arrow;
            float endAngle = juce::MathConstants<float>::twoPi - 0.7f;
            float arrowX = center.x + r * std::sin(endAngle);
            float arrowY = center.y - r * std::cos(endAngle);
            arrow.addTriangle(arrowX - 2.0f, arrowY - 3.5f,
                              arrowX + 2.5f, arrowY,
                              arrowX - 2.5f, arrowY + 2.0f);
            g.fillPath(arrow);
        }
    }

private:
    TransportIconType iconType;
    bool isPlaying{ false };
    bool isLoopActive{ false };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TransportIconButton)
};

class KeyDetectorComponent : public juce::Component, public juce::Timer, public juce::ChangeListener
{
public:
    KeyDetectorComponent(GraphManager& graphMgr);
    ~KeyDetectorComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void timerCallback() override;
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;

    void syncKeyToPitchPlugin();
    void showManualKeySelectMenu();
    void showBpmSettingsMenu();
    void applyKeyToAutoTune(int rootNote, KeyDetector::ScaleType scale, const juce::String& sourceName, bool showNotificationPopup = true);

private:
    GraphManager& graphManager;
    BeatPlayerAudioProcessor* beatPlayer{ nullptr };

    // Beat Player UI
    juce::TextButton loadBeatButton{ "LOAD BEAT" };
    TransportIconButton playPauseButton{ TransportIconType::PlayPause, "PlayPause" };
    TransportIconButton stopButton{ TransportIconType::Stop, "Stop" };
    TransportIconButton loopButton{ TransportIconType::Loop, "Loop" };
    juce::Slider positionSlider;
    juce::Label timeLabel;
    juce::Slider volumeSlider;
    juce::Label volumeLabel;
    class DuckingButton : public juce::TextButton
    {
    public:
        DuckingButton() : juce::TextButton("DUCK: OFF") {}

        std::function<void()> onToggle;
        std::function<void()> onRightClick;

        void clicked(const juce::ModifierKeys& modifiers) override
        {
            if (modifiers.isPopupMenu() || modifiers.isRightButtonDown())
            {
                if (onRightClick)
                    onRightClick();
            }
            else
            {
                if (onToggle)
                    onToggle();
            }
        }
    };

    class AiShieldButton : public juce::TextButton
    {
    public:
        AiShieldButton() : juce::TextButton(juce::String::fromUTF8(u8"🛡️ AI: OFF")) {}

        std::function<void()> onToggle;
        std::function<void()> onRightClick;

        void clicked(const juce::ModifierKeys& modifiers) override
        {
            if (modifiers.isPopupMenu() || modifiers.isRightButtonDown())
            {
                if (onRightClick)
                    onRightClick();
            }
            else
            {
                if (onToggle)
                    onToggle();
            }
        }
    };

    DuckingButton duckingButton;
    AiShieldButton aiShieldButton;
    juce::TextButton recButton{ "REC" };
    juce::TextButton recFolderButton{ "DIR" };

    // Key Detection UI
    juce::Label keyTitleLabel;
    juce::Label keyDisplayLabel;
    juce::Label confidenceLabel;
    juce::TextButton autoPushToggle;
    juce::TextButton syncToAutoTuneButton;
    juce::TextButton sourceToggleButton;
    juce::TextButton manualKeyButton;

    // Tempo (BPM) & Tap Tempo Buttons
    juce::TextButton bpmButton{ "120 BPM" };
    juce::TextButton tapTempoButton{ "TAP" };

    // File Chooser for Beat
    std::unique_ptr<juce::FileChooser> fileChooser;

    bool isDraggingPosition{ false };
    bool isAutoPushEnabled{ true };
    juce::String lastAutoPushedKey;
    float smoothedChroma[12]{ 0.0f };

    void handleLoadBeat();
    void togglePlayPause();
    void handleToggleRecord();
    void updateTransportUI();
    void updateKeyUI();
    void scanAutoKeyPluginsInRack();
    void updateAutoPushUI();
    void updateDuckingButtonUI();
    void showDuckingSettingsMenu();
    void updateAiShieldButtonUI();
    void showAiShieldSettingsMenu();
    void updateRecordButtonUI();

    juce::String lastAutoKeyPluginKey;
    double lastAutoKeyPluginBpm{ 0.0 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(KeyDetectorComponent)
};
