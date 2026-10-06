#include "VerticalRackComponent.h"

VerticalRackComponent::VerticalRackComponent(GraphManager& gm)
    : graphManager(gm)
{
    graphManager.addChangeListener(this);

    addSlotButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff1e293b));
    addSlotButton.setColour(juce::TextButton::textColourOffId, juce::Colour(0xff38bdf8));
    addSlotButton.onClick = [this]() {
        graphManager.addCustomSlot("Custom Effect", false);
        rebuildSlotUI();
    };
    rackContent.addAndMakeVisible(addSlotButton);

    viewport.setViewedComponent(&rackContent, false);
    viewport.setScrollBarsShown(true, false);
    addAndMakeVisible(viewport);

    rebuildSlotUI();
}

VerticalRackComponent::~VerticalRackComponent()
{
    graphManager.removeChangeListener(this);
}

void VerticalRackComponent::rebuildSlotUI()
{
    slotComponents.clear();

    const auto& slots = graphManager.getSlots();
    for (size_t i = 0; i < slots.size(); ++i)
    {
        auto slotComp = std::make_unique<PluginSlotComponent>(graphManager, static_cast<int>(i));
        slotComp->onDeleteSlotRequested = [this](int index) {
            graphManager.removeSlot(index);
            rebuildSlotUI();
        };
        rackContent.addAndMakeVisible(slotComp.get());
        slotComponents.push_back(std::move(slotComp));
    }

    rackContent.addAndMakeVisible(addSlotButton);
    resized();
}

void VerticalRackComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff0d0f15));

    // Rack guide rails
    g.setColour(juce::Colour(0xff1f2430));
    g.fillRect(0, 0, 8, getHeight());
    g.fillRect(getWidth() - 8, 0, 8, getHeight());
}

void VerticalRackComponent::paintOverChildren(juce::Graphics& g)
{
    if (dragTargetIndex >= 0 && dragTargetIndex <= static_cast<int>(slotComponents.size()))
    {
        const int slotHeight = 62;
        const int slotStep = slotHeight + 6;
        const int slotY = 6 + dragTargetIndex * slotStep - viewport.getViewPositionY() + viewport.getY();

        if (slotY >= viewport.getY() && slotY <= viewport.getBottom() + 10)
        {
            // Outer glow
            g.setColour(juce::Colour(0x6600e676));
            g.fillRoundedRectangle(16.0f, static_cast<float>(slotY - 4), static_cast<float>(getWidth() - 32), 8.0f, 4.0f);

            // Core indicator line
            g.setColour(juce::Colour(0xff00e676));
            g.fillRoundedRectangle(20.0f, static_cast<float>(slotY - 2), static_cast<float>(getWidth() - 40), 4.0f, 2.0f);

            // Left and right guide dots
            g.fillEllipse(12.0f, static_cast<float>(slotY - 5), 10.0f, 10.0f);
            g.fillEllipse(static_cast<float>(getWidth() - 22), static_cast<float>(slotY - 5), 10.0f, 10.0f);
        }
    }
}

bool VerticalRackComponent::isInterestedInDragSource(const SourceDetails& dragSourceDetails)
{
    return dragSourceDetails.description.toString().startsWith("SlotReorder:");
}

void VerticalRackComponent::itemDragEnter(const SourceDetails& dragSourceDetails)
{
    itemDragMove(dragSourceDetails);
}

void VerticalRackComponent::itemDragMove(const SourceDetails& dragSourceDetails)
{
    if (slotComponents.empty())
        return;

    const int viewY = viewport.getViewPositionY();
    const int mouseYInContent = dragSourceDetails.localPosition.y + viewY - viewport.getY();
    const int slotHeight = 62;
    const int slotStep = slotHeight + 6;

    int targetIdx = (mouseYInContent - 6 + (slotStep / 2)) / slotStep;
    targetIdx = std::clamp(targetIdx, 0, static_cast<int>(slotComponents.size()));

    if (dragTargetIndex != targetIdx)
    {
        dragTargetIndex = targetIdx;
        repaint();
    }
}

void VerticalRackComponent::itemDragExit(const SourceDetails& /*dragSourceDetails*/)
{
    dragTargetIndex = -1;
    repaint();
}

void VerticalRackComponent::itemDropped(const SourceDetails& dragSourceDetails)
{
    if (!slotComponents.empty())
    {
        const int viewY = viewport.getViewPositionY();
        const int mouseYInContent = dragSourceDetails.localPosition.y + viewY - viewport.getY();
        const int slotHeight = 62;
        const int slotStep = slotHeight + 6;

        int toIndex = (mouseYInContent - 6 + (slotStep / 2)) / slotStep;
        toIndex = std::clamp(toIndex, 0, static_cast<int>(slotComponents.size()) - 1);

        const juce::String desc = dragSourceDetails.description.toString();
        if (desc.startsWith("SlotReorder:"))
        {
            const int fromIndex = desc.fromFirstOccurrenceOf("SlotReorder:", false, false).getIntValue();
            if (fromIndex != toIndex)
            {
                graphManager.moveSlot(fromIndex, toIndex);
                rebuildSlotUI();
            }
        }
    }

    dragTargetIndex = -1;
    repaint();
}

void VerticalRackComponent::resized()
{
    auto bounds = getLocalBounds().reduced(8, 0);
    viewport.setBounds(bounds);

    const int slotHeight = 62;
    const int buttonHeight = 36;
    const int totalHeight = static_cast<int>(slotComponents.size()) * (slotHeight + 6) + buttonHeight + 20;

    rackContent.setBounds(0, 0, bounds.getWidth() - 14, totalHeight);

    int y = 6;
    for (auto& slotComp : slotComponents)
    {
        slotComp->setBounds(6, y, rackContent.getWidth() - 12, slotHeight);
        y += slotHeight + 6;
    }

    addSlotButton.setBounds(6, y + 4, rackContent.getWidth() - 12, buttonHeight);
}

void VerticalRackComponent::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    if (source == &graphManager)
    {
        updateAllSlots();
    }
}

void VerticalRackComponent::updateAllSlots()
{
    // If slot count changed, rebuild UI
    if (slotComponents.size() != graphManager.getSlots().size())
    {
        rebuildSlotUI();
        return;
    }

    for (auto& slotComp : slotComponents)
    {
        slotComp->updateSlotUI();
    }
}
