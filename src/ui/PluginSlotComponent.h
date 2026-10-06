#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "../audio/GraphManager.h"

class PluginSlotComponent;

class PluginWindow : public juce::DocumentWindow
{
public:
    PluginWindow(const juce::String& name, juce::AudioProcessorEditor* editor, std::function<void()> closeCallback)
        : DocumentWindow(name, juce::Colour(0xff12141c), DocumentWindow::closeButton),
          onClose(std::move(closeCallback))
    {
        setUsingNativeTitleBar(true);
        setContentOwned(editor, true);
        setResizable(editor->isResizable(), false);
        centreWithSize(editor->getWidth(), editor->getHeight());
        setVisible(true);
    }

    ~PluginWindow() override
    {
        clearContentComponent();
    }

    void closeButtonPressed() override
    {
        if (onClose)
            onClose();
    }

private:
    std::function<void()> onClose;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginWindow)
};

class PluginSlotComponent : public juce::Component
{
public:
    PluginSlotComponent(GraphManager& graphManager, int slotIndex);
    ~PluginSlotComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void mouseDoubleClick(const juce::MouseEvent& event) override;
    void mouseDown(const juce::MouseEvent& event) override;
    void mouseDrag(const juce::MouseEvent& event) override;

    void updateSlotUI();
    void openPluginWindow();

    std::function<void(int)> onDeleteSlotRequested;

private:
    GraphManager& graphManager;
    const int slotIndex;

    juce::Label dragGripLabel;
    juce::Label slotNumberLabel;
    juce::Label roleLabel;
    juce::Label pluginNameLabel;
    juce::TextButton routingTypeButton{ "INSERT" };

    // Send Level Controls (for Aux Send & Trim)
    juce::Slider sendLevelSlider;
    juce::Label sendLevelLabel;

    juce::ToggleButton bypassButton{ "BYPASS" };
    juce::TextButton editButton{ "EDIT" };
    juce::TextButton loadButton{ "LOAD VST3" };
    juce::TextButton removeButton{ "X" };

    std::unique_ptr<PluginWindow> activeEditorWindow;

    void showPluginMenu();
    void showRoleSelectionMenu();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginSlotComponent)
};
