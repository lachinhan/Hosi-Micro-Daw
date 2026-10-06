#include "PluginSlotComponent.h"

PluginSlotComponent::PluginSlotComponent(GraphManager& gm, int index)
    : graphManager(gm), slotIndex(index)
{
    // Drag Handle Grip
    dragGripLabel.setText(juce::CharPointer_UTF8("\xe2\xa0\xbf"), juce::dontSendNotification); // '⠿' 6-dot braille pattern
    dragGripLabel.setFont(juce::FontOptions(14.0f, juce::Font::bold));
    dragGripLabel.setColour(juce::Label::textColourId, juce::Colour(0xff64748b));
    dragGripLabel.setJustificationType(juce::Justification::centred);
    dragGripLabel.setTooltip("Click & Drag to reorder slot position");
    dragGripLabel.setMouseCursor(juce::MouseCursor::DraggingHandCursor);
    dragGripLabel.addMouseListener(this, false);
    addAndMakeVisible(dragGripLabel);

    // Slot Number indicator
    slotNumberLabel.setText(juce::String(slotIndex + 1), juce::dontSendNotification);
    slotNumberLabel.setFont(juce::FontOptions(15.0f, juce::Font::bold));
    slotNumberLabel.setColour(juce::Label::textColourId, juce::Colour(0xff00ffff));
    slotNumberLabel.setJustificationType(juce::Justification::centred);
    slotNumberLabel.setInterceptsMouseClicks(false, false);
    addAndMakeVisible(slotNumberLabel);

    // Slot Role / Custom Title (editable on click / double click)
    roleLabel.setFont(juce::FontOptions(13.0f, juce::Font::bold));
    roleLabel.setColour(juce::Label::textColourId, juce::Colour(0xffe0e6ed));
    roleLabel.setTooltip("Click or double-click to rename slot");
    roleLabel.setEditable(false, true, false);
    roleLabel.onTextChange = [this]() {
        const juce::String newName = roleLabel.getText().trim();
        if (newName.isNotEmpty())
        {
            graphManager.setSlotName(slotIndex, newName);
        }
    };
    addAndMakeVisible(roleLabel);

    // Plugin Name label
    pluginNameLabel.setFont(juce::FontOptions(12.0f, juce::Font::plain));
    pluginNameLabel.setColour(juce::Label::textColourId, juce::Colour(0xff8c9ba5));
    pluginNameLabel.setText("[Empty Slot - Click Load VST3]", juce::dontSendNotification);
    pluginNameLabel.setInterceptsMouseClicks(false, false);
    addAndMakeVisible(pluginNameLabel);

    // Routing Type Button (Toggle between INSERT Serial and AUX SEND Parallel)
    routingTypeButton.onClick = [this]() {
        const bool currentAux = graphManager.isSlotSpatialAux(slotIndex);
        graphManager.setSlotRoutingType(slotIndex, !currentAux);
        updateSlotUI();
    };
    addAndMakeVisible(routingTypeButton);

    // Send Level Rotary Knob / Slider (for Aux Send amount)
    sendLevelSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    sendLevelSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    sendLevelSlider.setRange(-60.0, 6.0, 0.1);
    sendLevelSlider.setValue(graphManager.getSlotGainDb(slotIndex), juce::dontSendNotification);
    sendLevelSlider.setDoubleClickReturnValue(true, 0.0);
    sendLevelSlider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xffa855f7)); // Purple
    sendLevelSlider.setColour(juce::Slider::thumbColourId, juce::Colour(0xffe9d5ff));

    sendLevelSlider.onValueChange = [this]() {
        const double db = sendLevelSlider.getValue();
        graphManager.setSlotGainDb(slotIndex, static_cast<float>(db));
        if (db <= -59.5)
            sendLevelLabel.setText("-inf", juce::dontSendNotification);
        else
            sendLevelLabel.setText(juce::String::formatted("%+.1fdB", db), juce::dontSendNotification);
    };
    addChildComponent(sendLevelSlider);

    sendLevelLabel.setText("0.0dB", juce::dontSendNotification);
    sendLevelLabel.setFont(juce::FontOptions(10.0f, juce::Font::bold));
    sendLevelLabel.setColour(juce::Label::textColourId, juce::Colour(0xffc084fc));
    sendLevelLabel.setJustificationType(juce::Justification::centred);
    addChildComponent(sendLevelLabel);

    // Bypass Toggle Checkbox
    bypassButton.setColour(juce::ToggleButton::textColourId, juce::Colour(0xffff9900));
    bypassButton.onClick = [this]() {
        graphManager.setSlotBypassed(slotIndex, bypassButton.getToggleState());
        repaint();
    };
    addAndMakeVisible(bypassButton);

    // Edit Button
    editButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff0284c7));
    editButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    editButton.onClick = [this]() { openPluginWindow(); };
    addAndMakeVisible(editButton);

    // Load VST3 Button
    loadButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff2d3748));
    loadButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    loadButton.onClick = [this]() { showPluginMenu(); };
    addAndMakeVisible(loadButton);

    // Remove / Clear Button
    removeButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff4a1525));
    removeButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    removeButton.onClick = [this]() {
        const auto node = graphManager.getSlotNode(slotIndex);
        const bool hasPlugin = (node != nullptr);
        const int totalSlots = static_cast<int>(graphManager.getSlots().size());

        if (hasPlugin)
        {
            activeEditorWindow.reset();
            graphManager.removePluginFromSlot(slotIndex);
            updateSlotUI();
        }
        else if (totalSlots > GraphManager::DEFAULT_SLOTS || slotIndex >= GraphManager::DEFAULT_SLOTS)
        {
            activeEditorWindow.reset();
            if (onDeleteSlotRequested)
                onDeleteSlotRequested(slotIndex);
        }
        else
        {
            activeEditorWindow.reset();
            graphManager.removePluginFromSlot(slotIndex);
            updateSlotUI();
        }
    };
    addAndMakeVisible(removeButton);

    updateSlotUI();
}

PluginSlotComponent::~PluginSlotComponent()
{
    activeEditorWindow.reset();
}

void PluginSlotComponent::updateSlotUI()
{
    const auto& slots = graphManager.getSlots();
    if (slotIndex < static_cast<int>(slots.size()))
    {
        const auto& slot = slots[slotIndex];
        roleLabel.setText(slot.slotName, juce::dontSendNotification);

        // Update Routing Type Badge & Send Level Visibility
        if (slot.isSpatialAux)
        {
            routingTypeButton.setButtonText("AUX SEND");
            routingTypeButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff581c87)); // Deep Purple
            routingTypeButton.setColour(juce::TextButton::textColourOffId, juce::Colour(0xffe9d5ff));

            sendLevelSlider.setVisible(true);
            sendLevelLabel.setVisible(true);
            sendLevelSlider.setValue(slot.sendGainDb, juce::dontSendNotification);
            if (slot.sendGainDb <= -59.5f)
                sendLevelLabel.setText("-inf", juce::dontSendNotification);
            else
                sendLevelLabel.setText(juce::String::formatted("%+.1fdB", slot.sendGainDb), juce::dontSendNotification);
        }
        else
        {
            routingTypeButton.setButtonText("INSERT");
            routingTypeButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff0f766e)); // Teal
            routingTypeButton.setColour(juce::TextButton::textColourOffId, juce::Colour(0xff99f6e4));

            sendLevelSlider.setVisible(false);
            sendLevelLabel.setVisible(false);
        }

        bypassButton.setToggleState(slot.isBypassed, juce::dontSendNotification);

        if (slot.node != nullptr && slot.node->getProcessor() != nullptr)
        {
            pluginNameLabel.setText(slot.node->getProcessor()->getName(), juce::dontSendNotification);
            pluginNameLabel.setColour(juce::Label::textColourId, juce::Colour(0xff00ffcc));
            loadButton.setButtonText("CHANGE");
            editButton.setEnabled(true);
            bypassButton.setEnabled(true);
            removeButton.setEnabled(true);
        }
        else
        {
            pluginNameLabel.setText("[No Plugin Loaded - Double-click or Load]", juce::dontSendNotification);
            pluginNameLabel.setColour(juce::Label::textColourId, juce::Colour(0xff718096));
            loadButton.setButtonText("LOAD VST3");
            editButton.setEnabled(false);
            bypassButton.setEnabled(true);
            removeButton.setEnabled(true);
        }
    }
    resized();
    repaint();
}

void PluginSlotComponent::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat().reduced(2.0f);

    const bool isBypassed = graphManager.isSlotBypassed(slotIndex);
    const auto node = graphManager.getSlotNode(slotIndex);
    const bool hasPlugin = (node != nullptr);

    // Rack chassis background
    juce::Colour bgColour = hasPlugin 
        ? (isBypassed ? juce::Colour(0xff181c24) : juce::Colour(0xff141e2e))
        : (isBypassed ? juce::Colour(0xff16161c) : juce::Colour(0xff12151c));

    g.setColour(bgColour);
    g.fillRoundedRectangle(bounds, 6.0f);

    // Border highlight
    juce::Colour borderColour = hasPlugin
        ? (isBypassed ? juce::Colour(0xff4a5568) : juce::Colour(0xff00bcd4))
        : juce::Colour(0xff2d3748);

    g.setColour(borderColour);
    g.drawRoundedRectangle(bounds, 6.0f, 1.2f);

    // LED indicator
    auto ledArea = juce::Rectangle<float>(12.0f, bounds.getCentreY() - 4.0f, 8.0f, 8.0f);
    if (hasPlugin && !isBypassed)
        g.setColour(juce::Colour(0xff00e676)); // Active Green LED
    else if (isBypassed)
        g.setColour(juce::Colour(0xffff9100)); // Bypassed Orange LED
    else
        g.setColour(juce::Colour(0xff374151)); // Off LED

    g.fillEllipse(ledArea);
}

void PluginSlotComponent::resized()
{
    auto bounds = getLocalBounds().reduced(6, 4);

    // Left Drag Handle Grip
    dragGripLabel.setBounds(bounds.removeFromLeft(16));

    // LED spacing & Slot Number
    bounds.removeFromLeft(14); // space for LED indicator
    slotNumberLabel.setBounds(bounds.removeFromLeft(24));

    // Right action buttons
    removeButton.setBounds(bounds.removeFromRight(26).reduced(2));
    loadButton.setBounds(bounds.removeFromRight(88).reduced(2));
    editButton.setBounds(bounds.removeFromRight(60).reduced(2));
    bypassButton.setBounds(bounds.removeFromRight(76).reduced(2));

    const bool isAux = graphManager.isSlotSpatialAux(slotIndex);
    if (isAux)
    {
        // Aux Send knob + label area (56px)
        auto sendArea = bounds.removeFromRight(56);
        sendLevelLabel.setBounds(sendArea.removeFromBottom(16));
        sendLevelSlider.setBounds(sendArea.reduced(2));
    }

    routingTypeButton.setBounds(bounds.removeFromRight(80).reduced(2, 6));

    // Middle info
    auto infoArea = bounds.reduced(4, 0);
    roleLabel.setBounds(infoArea.removeFromTop(infoArea.getHeight() / 2));
    pluginNameLabel.setBounds(infoArea);
}

void PluginSlotComponent::mouseDown(const juce::MouseEvent& event)
{
    if (event.mods.isRightButtonDown())
    {
        showRoleSelectionMenu();
    }
}

void PluginSlotComponent::mouseDrag(const juce::MouseEvent& event)
{
    if (event.getDistanceFromDragStart() > 6)
    {
        if (auto* dragContainer = juce::DragAndDropContainer::findParentDragContainerFor(this))
        {
            if (!dragContainer->isDragAndDropActive())
            {
                auto snap = createComponentSnapshot(getLocalBounds(), true, 1.0f);
                dragContainer->startDragging("SlotReorder:" + juce::String(slotIndex), this, juce::ScaledImage(snap), true);
            }
        }
    }
}

void PluginSlotComponent::mouseDoubleClick(const juce::MouseEvent& /*event*/)
{
    openPluginWindow();
}

void PluginSlotComponent::showRoleSelectionMenu()
{
    juce::PopupMenu menu;
    menu.addSectionHeader("Select Slot Function / Preset Role");
    menu.addItem(1, "Pitch Correction / Auto-Tune");
    menu.addItem(2, "Noise Gate");
    menu.addItem(3, "Subtractive EQ");
    menu.addItem(4, "Vocal Compressor (Peak / Opto)");
    menu.addItem(5, "De-Esser");
    menu.addItem(6, "Vocal Saturation / Warmth");
    menu.addItem(7, "Chorus / Doubler");
    menu.addItem(8, "Spatial Reverb (Parallel)");
    menu.addItem(9, "Spatial Delay / Echo (Parallel)");
    menu.addItem(10, "Custom Insert Effect");

    menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(&roleLabel),
        [this](int result) {
            if (result == 1) { graphManager.setSlotName(slotIndex, "Pitch Correction / Auto-Tune"); graphManager.setSlotRoutingType(slotIndex, false); }
            else if (result == 2) { graphManager.setSlotName(slotIndex, "Noise Gate"); graphManager.setSlotRoutingType(slotIndex, false); }
            else if (result == 3) { graphManager.setSlotName(slotIndex, "Subtractive EQ"); graphManager.setSlotRoutingType(slotIndex, false); }
            else if (result == 4) { graphManager.setSlotName(slotIndex, "Vocal Compressor"); graphManager.setSlotRoutingType(slotIndex, false); }
            else if (result == 5) { graphManager.setSlotName(slotIndex, "De-Esser"); graphManager.setSlotRoutingType(slotIndex, false); }
            else if (result == 6) { graphManager.setSlotName(slotIndex, "Vocal Saturation"); graphManager.setSlotRoutingType(slotIndex, false); }
            else if (result == 7) { graphManager.setSlotName(slotIndex, "Chorus / Doubler"); graphManager.setSlotRoutingType(slotIndex, false); }
            else if (result == 8) { graphManager.setSlotName(slotIndex, "Spatial Reverb"); graphManager.setSlotRoutingType(slotIndex, true); }
            else if (result == 9) { graphManager.setSlotName(slotIndex, "Spatial Delay"); graphManager.setSlotRoutingType(slotIndex, true); }
            else if (result == 10) { graphManager.setSlotName(slotIndex, "Custom Insert"); }
            updateSlotUI();
        });
}

void PluginSlotComponent::openPluginWindow()
{
    auto node = graphManager.getSlotNode(slotIndex);
    if (node != nullptr && node->getProcessor() != nullptr)
    {
        if (activeEditorWindow != nullptr)
        {
            activeEditorWindow->setVisible(true);
            activeEditorWindow->toFront(true);
            return;
        }

        auto* processor = node->getProcessor();
        juce::AudioProcessorEditor* editor = processor->hasEditor() ? processor->createEditorIfNeeded() : nullptr;
        if (editor == nullptr && processor->hasEditor())
        {
            editor = processor->createEditor();
        }

        if (editor != nullptr)
        {
            activeEditorWindow = std::make_unique<PluginWindow>(
                processor->getName(),
                editor,
                [this]() { activeEditorWindow.reset(); }
            );
        }
        else
        {
            juce::AlertWindow::showMessageBoxAsync(
                juce::AlertWindow::InfoIcon,
                "Plugin Editor",
                "This plugin does not provide a custom graphical editor.",
                "OK"
            );
        }
    }
    else
    {
        showPluginMenu();
    }
}

void PluginSlotComponent::showPluginMenu()
{
    juce::File defaultVst3Dir("C:\\Program Files\\Common Files\\VST3");
    if (!defaultVst3Dir.exists())
    {
        defaultVst3Dir = juce::File("C:\\Program Files (x86)\\Common Files\\VST3");
    }

    auto fileChooser = std::make_shared<juce::FileChooser>(
        "Select VST3 Plugin",
        defaultVst3Dir,
        "*.vst3"
    );

    fileChooser->launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles | juce::FileBrowserComponent::canSelectDirectories,
        [this, fileChooser](const juce::FileChooser& fc)
        {
            auto file = fc.getResult();
            if (file.exists())
            {
                juce::OwnedArray<juce::PluginDescription> descriptions;
                juce::VST3PluginFormat format;
                format.findAllTypesForFile(descriptions, file.getFullPathName());

                if (!descriptions.isEmpty())
                {
                    juce::String error;
                    if (graphManager.loadPluginIntoSlot(slotIndex, *descriptions[0], error))
                    {
                        updateSlotUI();
                        openPluginWindow(); // Automatically pop open GUI
                    }
                    else
                    {
                        juce::AlertWindow::showMessageBoxAsync(
                            juce::AlertWindow::WarningIcon,
                            "Error Loading VST3",
                            "Failed to initialize plugin: " + error,
                            "OK"
                        );
                    }
                }
            }
        });
}
