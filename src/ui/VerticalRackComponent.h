#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../audio/GraphManager.h"
#include "PluginSlotComponent.h"
#include <vector>
#include <memory>

class VerticalRackComponent : public juce::Component, 
                              public juce::ChangeListener,
                              public juce::DragAndDropContainer,
                              public juce::DragAndDropTarget
{
public:
    explicit VerticalRackComponent(GraphManager& graphManager);
    ~VerticalRackComponent() override;

    void paint(juce::Graphics& g) override;
    void paintOverChildren(juce::Graphics& g) override;
    void resized() override;
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;

    // Drag and Drop Target implementation
    bool isInterestedInDragSource(const SourceDetails& dragSourceDetails) override;
    void itemDragEnter(const SourceDetails& dragSourceDetails) override;
    void itemDragMove(const SourceDetails& dragSourceDetails) override;
    void itemDragExit(const SourceDetails& dragSourceDetails) override;
    void itemDropped(const SourceDetails& dragSourceDetails) override;

    void rebuildSlotUI();
    void updateAllSlots();

private:
    GraphManager& graphManager;

    juce::Viewport viewport;
    juce::Component rackContent;
    std::vector<std::unique_ptr<PluginSlotComponent>> slotComponents;
    juce::TextButton addSlotButton{ "+ ADD CUSTOM PLUGIN SLOT" };

    int dragTargetIndex{ -1 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VerticalRackComponent)
};
