#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../audio/SoundboardAudioProcessor.h"
#include "../audio/GraphManager.h"

class SoundPadButton : public juce::Button
{
public:
    SoundPadButton(int padIdx, SoundboardAudioProcessor& proc);
    ~SoundPadButton() override = default;

    void paintButton(juce::Graphics& g, bool isMouseOverButton, bool isButtonDown) override;
    void clicked() override;
    void clicked(const juce::ModifierKeys& modifiers) override;
    void mouseDoubleClick(const juce::MouseEvent& e) override;

    void showRenameDialog();
    void showContextMenu();
    void loadCustomSample();
    void resetToDefault();

private:
    int padIndex{ 0 };
    SoundboardAudioProcessor& processor;
    std::unique_ptr<juce::FileChooser> fileChooser;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SoundPadButton)
};

class SoundboardComponent : public juce::Component, public juce::Timer, public juce::ChangeListener
{
public:
    SoundboardComponent(GraphManager& graphMgr);
    ~SoundboardComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void timerCallback() override;
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;

    void triggerPad(int padIndex);
    void stopAll();

private:
    GraphManager& graphManager;
    SoundboardAudioProcessor* soundboardProcessor{ nullptr };

    juce::Label titleLabel;
    juce::TextButton stopAllButton{ "STOP ALL" };
    juce::Slider volumeSlider;
    juce::Label volumeLabel;

    std::vector<std::unique_ptr<SoundPadButton>> padButtons;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SoundboardComponent)
};
