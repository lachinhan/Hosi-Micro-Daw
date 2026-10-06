#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../audio/GraphManager.h"
#include "PluginSlotComponent.h"
#include <map>

// Individual Master Insert Slot Row Component
class MasterSlotItemComponent : public juce::Component
{
public:
    MasterSlotItemComponent(GraphManager& gm, int slotIndex);
    ~MasterSlotItemComponent() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void updateUI();

    std::function<void(int)> onOpenEditor;
    std::function<void(int)> onRemoveSlot;

    int getSlotIndex() const noexcept { return slotIndex; }

private:
    GraphManager& graphManager;
    int slotIndex{ 0 };

    juce::Label slotTitleLabel;
    juce::Label pluginNameLabel;
    juce::TextButton loadButton{ "LOAD VST3" };
    juce::TextButton editButton{ "EDIT" };
    juce::TextButton bypassButton{ "BYPASS" };
    juce::TextButton removeButton{ "X" };

    void showPluginChooser();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MasterSlotItemComponent)
};

class MasterChannelStripComponent : public juce::Component, public juce::Timer, public juce::ChangeListener
{
public:
    explicit MasterChannelStripComponent(GraphManager& graphManager);
    ~MasterChannelStripComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void timerCallback() override;
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;

    void rebuildMasterSlotsUI();
    void updateMasterSlotsUI();
    void updateFaderUI();
    void updateAllUI();
    void openMasterPluginWindow(int slotIndex);

private:
    GraphManager& graphManager;

    // Master Inserts Section Header
    juce::Label masterInsertsTitleLabel;
    juce::TextButton addMasterSlotButton{ "+ ADD MASTER SLOT" };

    // Viewport and Container for Multi Master Slots
    juce::Viewport masterSlotsViewport;
    std::unique_ptr<juce::Component> masterSlotsContainer;
    std::vector<std::unique_ptr<MasterSlotItemComponent>> masterSlotComponents;

    std::map<int, std::unique_ptr<PluginWindow>> activePluginWindows;

    // Master Fader & Metering UI
    juce::Label titleLabel;
    juce::Slider masterFaderSlider;
    juce::Label faderDbLabel;
    juce::TextButton reset0DbButton{ "0 dB" };

    float currentLeftPeak{ 0.0f };
    float currentRightPeak{ 0.0f };
    bool isClipping{ false };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MasterChannelStripComponent)
};
