#include "MasterChannelStripComponent.h"
#include <cmath>

// ==============================================================================
// MasterSlotItemComponent
// ==============================================================================
MasterSlotItemComponent::MasterSlotItemComponent(GraphManager& gm, int index)
    : graphManager(gm), slotIndex(index)
{
    // Slot Title Label (Editable)
    slotTitleLabel.setFont(juce::FontOptions(11.0f, juce::Font::bold));
    slotTitleLabel.setColour(juce::Label::textColourId, juce::Colour(0xfff59e0b)); // Amber Gold
    slotTitleLabel.setEditable(false, true, false);
    slotTitleLabel.onTextChange = [this]() {
        graphManager.setMasterSlotName(slotIndex, slotTitleLabel.getText());
    };
    addAndMakeVisible(slotTitleLabel);

    // Plugin Name Label
    pluginNameLabel.setFont(juce::FontOptions(10.0f, juce::Font::plain));
    pluginNameLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(pluginNameLabel);

    // Load VST3 Button
    loadButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff27272a));
    loadButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    loadButton.onClick = [this]() { showPluginChooser(); };
    addAndMakeVisible(loadButton);

    // Edit GUI Button
    editButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff0284c7));
    editButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    editButton.onClick = [this]() {
        if (onOpenEditor)
            onOpenEditor(slotIndex);
    };
    addAndMakeVisible(editButton);

    // Bypass Button (Clean TextButton with active glow)
    bypassButton.setButtonText("BYPASS");
    bypassButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff1e293b));
    bypassButton.setColour(juce::TextButton::textColourOffId, juce::Colour(0xff94a3b8));
    bypassButton.onClick = [this]() {
        const bool currentBypass = graphManager.isMasterSlotBypassed(slotIndex);
        graphManager.setMasterSlotBypassed(slotIndex, !currentBypass);
        updateUI();
    };
    addAndMakeVisible(bypassButton);

    // Remove Slot / Plugin Button
    removeButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff4a1525));
    removeButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    removeButton.onClick = [this]() {
        if (onRemoveSlot)
            onRemoveSlot(slotIndex);
    };
    addAndMakeVisible(removeButton);

    updateUI();
}

void MasterSlotItemComponent::updateUI()
{
    const auto& masterSlots = graphManager.getMasterSlots();
    if (slotIndex >= 0 && slotIndex < static_cast<int>(masterSlots.size()))
    {
        const auto& slotData = masterSlots[static_cast<size_t>(slotIndex)];
        slotTitleLabel.setText("M" + juce::String(slotIndex + 1) + ": " + slotData.slotName, juce::dontSendNotification);

        const juce::String pluginName = graphManager.getMasterSlotPluginName(slotIndex);
        const bool hasPlugin = pluginName.isNotEmpty();

        if (hasPlugin)
        {
            pluginNameLabel.setText(pluginName, juce::dontSendNotification);
            pluginNameLabel.setColour(juce::Label::textColourId, juce::Colour(0xff38bdf8)); // Sky Blue
            loadButton.setButtonText("CHG");
            loadButton.setTooltip("Change VST3 Plugin");
            editButton.setEnabled(true);
            bypassButton.setEnabled(true);

            if (slotData.isBypassed)
            {
                bypassButton.setButtonText("BYPASSED");
                bypassButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xffd97706)); // Orange
                bypassButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
                bypassButton.setTooltip("Plugin is Bypassed (Muted) - Click to Enable");
            }
            else
            {
                bypassButton.setButtonText("BYPASS");
                bypassButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff1e293b));
                bypassButton.setColour(juce::TextButton::textColourOffId, juce::Colour(0xff94a3b8));
                bypassButton.setTooltip("Click to Bypass plugin");
            }
        }
        else
        {
            pluginNameLabel.setText("[Empty Slot]", juce::dontSendNotification);
            pluginNameLabel.setColour(juce::Label::textColourId, juce::Colour(0xff64748b)); // Slate
            loadButton.setButtonText("LOAD");
            loadButton.setTooltip("Load VST3 Plugin into Master Slot");
            editButton.setEnabled(false);
            bypassButton.setEnabled(false);
            bypassButton.setButtonText("BYPASS");
            bypassButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff181c26));
            bypassButton.setColour(juce::TextButton::textColourOffId, juce::Colour(0xff475569));
            bypassButton.setTooltip("No plugin to bypass");
        }
    }
    repaint();
}

void MasterSlotItemComponent::showPluginChooser()
{
    juce::File defaultVst3Dir("C:\\Program Files\\Common Files\\VST3");
    if (!defaultVst3Dir.exists())
    {
        defaultVst3Dir = juce::File("C:\\Program Files (x86)\\Common Files\\VST3");
    }

    auto fileChooser = std::make_shared<juce::FileChooser>(
        "Select Master VST3 Plugin",
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
                    if (graphManager.loadMasterPluginIntoSlot(slotIndex, *descriptions[0], error))
                    {
                        updateUI();
                        if (onOpenEditor)
                            onOpenEditor(slotIndex);
                    }
                    else
                    {
                        juce::AlertWindow::showMessageBoxAsync(
                            juce::AlertWindow::WarningIcon,
                            "Error Loading Plugin",
                            "Failed to load VST3 into master slot: " + error,
                            "OK"
                        );
                    }
                }
            }
        });
}

void MasterSlotItemComponent::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat().reduced(1.0f);

    // Card background
    g.setColour(juce::Colour(0xff181c26));
    g.fillRoundedRectangle(bounds, 5.0f);

    const bool hasPlugin = graphManager.getMasterSlotPluginName(slotIndex).isNotEmpty();
    g.setColour(hasPlugin ? juce::Colour(0xff3f3f46) : juce::Colour(0xff27272a));
    g.drawRoundedRectangle(bounds, 5.0f, 1.0f);
}

void MasterSlotItemComponent::resized()
{
    auto bounds = getLocalBounds().reduced(4);

    // Top Header: Title + Remove Button
    auto topRow = bounds.removeFromTop(18);
    removeButton.setBounds(topRow.removeFromRight(18).reduced(1));
    slotTitleLabel.setBounds(topRow);

    // Middle: Plugin name
    pluginNameLabel.setBounds(bounds.removeFromTop(16));

    // Bottom Action Buttons: [LOAD/CHG] [EDIT] [BYPASS]
    auto bottomRow = bounds.removeFromTop(22);
    bypassButton.setBounds(bottomRow.removeFromRight(60).reduced(1, 0));
    editButton.setBounds(bottomRow.removeFromRight(42).reduced(1, 0));
    loadButton.setBounds(bottomRow.reduced(1, 0));
}

// ==============================================================================
// MasterChannelStripComponent
// ==============================================================================
MasterChannelStripComponent::MasterChannelStripComponent(GraphManager& gm)
    : graphManager(gm)
{
    graphManager.addChangeListener(this);

    // Master Inserts Section Title
    masterInsertsTitleLabel.setText("MASTER INSERTS", juce::dontSendNotification);
    masterInsertsTitleLabel.setFont(juce::FontOptions(11.0f, juce::Font::bold));
    masterInsertsTitleLabel.setColour(juce::Label::textColourId, juce::Colour(0xfff59e0b)); // Amber Gold
    masterInsertsTitleLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(masterInsertsTitleLabel);

    // Multi Master Slots Viewport & Container
    masterSlotsContainer = std::make_unique<juce::Component>();
    masterSlotsViewport.setViewedComponent(masterSlotsContainer.get(), false);
    masterSlotsViewport.setScrollBarsShown(true, false);
    addAndMakeVisible(masterSlotsViewport);

    // Add Master Slot Button
    addMasterSlotButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff164e63));
    addMasterSlotButton.setColour(juce::TextButton::textColourOffId, juce::Colour(0xff67e8f9));
    addMasterSlotButton.setTooltip("Add a new plugin slot to the Master Bus chain");
    addMasterSlotButton.onClick = [this]() {
        const int newIndex = static_cast<int>(graphManager.getMasterSlots().size()) + 1;
        graphManager.addMasterSlot("Master Insert " + juce::String(newIndex));
        rebuildMasterSlotsUI();
    };
    addAndMakeVisible(addMasterSlotButton);

    // ==========================================
    // Master Fader & Metering UI
    // ==========================================
    titleLabel.setText("MASTER OUT", juce::dontSendNotification);
    titleLabel.setFont(juce::FontOptions(12.0f, juce::Font::bold));
    titleLabel.setColour(juce::Label::textColourId, juce::Colour(0xff38bdf8));
    titleLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(titleLabel);

    // Master Fader: Range -60.0 dB to +6.0 dB, default 0.0 dB
    masterFaderSlider.setSliderStyle(juce::Slider::LinearVertical);
    masterFaderSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    masterFaderSlider.setRange(-60.0, 6.0, 0.1);
    masterFaderSlider.setValue(0.0, juce::dontSendNotification);
    masterFaderSlider.setDoubleClickReturnValue(true, 0.0);
    masterFaderSlider.setColour(juce::Slider::trackColourId, juce::Colour(0xff0284c7));
    masterFaderSlider.setColour(juce::Slider::backgroundColourId, juce::Colour(0xff090d16));
    masterFaderSlider.setColour(juce::Slider::thumbColourId, juce::Colour(0xff38bdf8));

    masterFaderSlider.onValueChange = [this]() {
        const double db = masterFaderSlider.getValue();
        const float linearGain = (db <= -59.5) ? 0.0f : static_cast<float>(std::pow(10.0, db / 20.0));
        graphManager.setMasterGain(linearGain);

        if (db <= -59.5)
            faderDbLabel.setText("-inf dB", juce::dontSendNotification);
        else
            faderDbLabel.setText(juce::String::formatted("%+.1f dB", db), juce::dontSendNotification);
    };
    addAndMakeVisible(masterFaderSlider);

    faderDbLabel.setText("+0.0 dB", juce::dontSendNotification);
    faderDbLabel.setFont(juce::FontOptions(13.0f, juce::Font::bold));
    faderDbLabel.setColour(juce::Label::textColourId, juce::Colour(0xff38bdf8)); // Cyan readout
    faderDbLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(faderDbLabel);

    reset0DbButton.setButtonText("0 dB RESET");
    reset0DbButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff1e293b));
    reset0DbButton.setColour(juce::TextButton::textColourOffId, juce::Colour(0xff94a3b8));
    reset0DbButton.onClick = [this]() {
        masterFaderSlider.setValue(0.0);
    };
    addAndMakeVisible(reset0DbButton);

    rebuildMasterSlotsUI();
    startTimerHz(30);
}

MasterChannelStripComponent::~MasterChannelStripComponent()
{
    stopTimer();
    graphManager.removeChangeListener(this);
    activePluginWindows.clear();
}

void MasterChannelStripComponent::changeListenerCallback(juce::ChangeBroadcaster* /*source*/)
{
    updateMasterSlotsUI();
}

void MasterChannelStripComponent::rebuildMasterSlotsUI()
{
    masterSlotComponents.clear();
    if (masterSlotsContainer == nullptr)
        return;

    masterSlotsContainer->removeAllChildren();

    const auto& masterSlots = graphManager.getMasterSlots();
    const int slotHeight = 66;
    const int gap = 4;
    int currentY = 0;

    int containerW = masterSlotsViewport.getViewWidth();
    if (containerW <= 20)
        containerW = masterSlotsViewport.getWidth() - 14;
    if (containerW <= 20)
        containerW = 172;

    for (size_t i = 0; i < masterSlots.size(); ++i)
    {
        const int idx = static_cast<int>(i);
        auto slotComp = std::make_unique<MasterSlotItemComponent>(graphManager, idx);
        slotComp->onOpenEditor = [this](int sIdx) { openMasterPluginWindow(sIdx); };
        slotComp->onRemoveSlot = [this](int sIdx) {
            activePluginWindows.erase(sIdx);
            if (graphManager.getMasterSlots().size() > 1)
            {
                graphManager.removeMasterSlot(sIdx);
            }
            else
            {
                graphManager.removePluginFromMasterSlot(sIdx);
            }
            rebuildMasterSlotsUI();
        };

        slotComp->setBounds(0, currentY, containerW, slotHeight);
        masterSlotsContainer->addAndMakeVisible(slotComp.get());
        masterSlotComponents.push_back(std::move(slotComp));

        currentY += slotHeight + gap;
    }

    masterSlotsContainer->setSize(containerW, std::max(currentY, slotHeight));
    resized();
    repaint();
}

void MasterChannelStripComponent::updateMasterSlotsUI()
{
    if (masterSlotComponents.size() != graphManager.getMasterSlots().size())
    {
        rebuildMasterSlotsUI();
        return;
    }

    for (auto& comp : masterSlotComponents)
    {
        if (comp != nullptr)
            comp->updateUI();
    }
}

void MasterChannelStripComponent::updateFaderUI()
{
    const float linear = graphManager.getMasterGain();
    const double db = (linear <= 0.0001f) ? -60.0 : 20.0 * std::log10(static_cast<double>(linear));
    masterFaderSlider.setValue(db, juce::dontSendNotification);

    if (db <= -59.5)
        faderDbLabel.setText("-inf dB", juce::dontSendNotification);
    else
        faderDbLabel.setText(juce::String::formatted("%+.1f dB", db), juce::dontSendNotification);
}

void MasterChannelStripComponent::updateAllUI()
{
    rebuildMasterSlotsUI();
    updateFaderUI();
}

void MasterChannelStripComponent::openMasterPluginWindow(int slotIndex)
{
    auto node = graphManager.getMasterSlotNode(slotIndex);
    if (node != nullptr && node->getProcessor() != nullptr)
    {
        auto it = activePluginWindows.find(slotIndex);
        if (it != activePluginWindows.end() && it->second != nullptr)
        {
            it->second->setVisible(true);
            it->second->toFront(true);
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
            activePluginWindows[slotIndex] = std::make_unique<PluginWindow>(
                "Master " + juce::String(slotIndex + 1) + ": " + processor->getName(),
                editor,
                [this, slotIndex]() {
                    activePluginWindows.erase(slotIndex);
                }
            );
        }
    }
}

void MasterChannelStripComponent::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat().reduced(2.0f);

    // Master Strip Card Outer Background
    g.setColour(juce::Colour(0xff12151f));
    g.fillRoundedRectangle(bounds, 8.0f);

    g.setColour(juce::Colour(0xff1e293b));
    g.drawRoundedRectangle(bounds, 8.0f, 1.2f);

    // If master fader is visible, render the balanced meters, clip LEDs, dB scale and fader well
    if (masterFaderSlider.isVisible() && masterFaderSlider.getHeight() > 30)
    {
        const auto faderBounds = masterFaderSlider.getBounds().toFloat();
        const float meterTop = faderBounds.getY() + 4.0f;
        const float meterHeight = faderBounds.getHeight() - 8.0f;
        const float meterBottom = meterTop + meterHeight;

        const float meterWidth = 9.0f;
        const float meterSpacing = 4.0f;
        const float dualMeterW = (meterWidth * 2.0f) + meterSpacing; // 22.0f
        
        // Group geometry: Meter (22px) + Gap (5px) + Scale (26px) + Gap (5px) + Fader (44px) = 102px total
        const float groupW = 102.0f;
        const float totalW = static_cast<float>(getWidth());
        const float startX = std::max(6.0f, (totalW - groupW) * 0.5f);

        const float leftMeterX = startX;
        const float rightMeterX = leftMeterX + meterWidth + meterSpacing;
        const float scaleX = rightMeterX + meterWidth + 5.0f;
        const float scaleW = 26.0f;

        // Clip Warning LEDs above meters (L & R)
        const float clipLedY = meterTop - 9.0f;
        g.setColour((currentLeftPeak >= 1.0f) ? juce::Colour(0xffef4444) : juce::Colour(0xff27272a));
        g.fillRoundedRectangle(leftMeterX, clipLedY, meterWidth, 5.0f, 1.5f);

        g.setColour((currentRightPeak >= 1.0f) ? juce::Colour(0xffef4444) : juce::Colour(0xff27272a));
        g.fillRoundedRectangle(rightMeterX, clipLedY, meterWidth, 5.0f, 1.5f);

        // Meter Background tracks with subtle border
        g.setColour(juce::Colour(0xff090d16));
        g.fillRoundedRectangle(leftMeterX, meterTop, meterWidth, meterHeight, 3.0f);
        g.fillRoundedRectangle(rightMeterX, meterTop, meterWidth, meterHeight, 3.0f);
        g.setColour(juce::Colour(0xff1e293b));
        g.drawRoundedRectangle(leftMeterX, meterTop, meterWidth, meterHeight, 3.0f, 0.8f);
        g.drawRoundedRectangle(rightMeterX, meterTop, meterWidth, meterHeight, 3.0f, 0.8f);

        // Convert linear peak amplitude to normalized dB height matching [-60 dB, +6 dB]
        auto getDbNorm = [](float peak) -> float {
            if (peak <= 0.001f) return 0.0f;
            const float db = 20.0f * std::log10(peak);
            return std::clamp((db - (-60.0f)) / (6.0f - (-60.0f)), 0.0f, 1.0f);
        };

        const float lNorm = getDbNorm(currentLeftPeak);
        const float lHeight = lNorm * meterHeight;
        if (lHeight > 0.0f)
        {
            juce::Colour lColor = (currentLeftPeak >= 1.0f) ? juce::Colour(0xffef4444)
                                : (currentLeftPeak > 0.50f) ? juce::Colour(0xfff59e0b) // > -6dB amber
                                : juce::Colour(0xff10b981); // green
            g.setColour(lColor);
            g.fillRoundedRectangle(leftMeterX + 1.0f, meterBottom - lHeight, meterWidth - 2.0f, lHeight, 2.0f);
        }

        const float rNorm = getDbNorm(currentRightPeak);
        const float rHeight = rNorm * meterHeight;
        if (rHeight > 0.0f)
        {
            juce::Colour rColor = (currentRightPeak >= 1.0f) ? juce::Colour(0xffef4444)
                                : (currentRightPeak > 0.50f) ? juce::Colour(0xfff59e0b)
                                : juce::Colour(0xff10b981);
            g.setColour(rColor);
            g.fillRoundedRectangle(rightMeterX + 1.0f, meterBottom - rHeight, meterWidth - 2.0f, rHeight, 2.0f);
        }

        // L and R labels at bottom of meters
        g.setColour(juce::Colour(0xff64748b));
        g.setFont(juce::FontOptions(8.5f, juce::Font::bold));
        g.drawText("L", static_cast<int>(leftMeterX), static_cast<int>(meterBottom + 2), static_cast<int>(meterWidth), 10, juce::Justification::centred);
        g.drawText("R", static_cast<int>(rightMeterX), static_cast<int>(meterBottom + 2), static_cast<int>(meterWidth), 10, juce::Justification::centred);

        // Centered dB Reference Scale (Ticks + Labels cleanly placed between Meter & Fader)
        g.setFont(juce::FontOptions(8.5f, juce::Font::plain));
        
        struct DbMark { float db; const char* label; juce::Colour col; };
        const std::array<DbMark, 6> marks = {{
            {  6.0f, "+6",  juce::Colour(0xffef4444) },
            {  0.0f, "0",   juce::Colour(0xff38bdf8) }, // 0dB highlighted in cyan
            { -6.0f, "-6",  juce::Colour(0xff94a3b8) },
            { -12.0f, "-12", juce::Colour(0xff64748b) },
            { -24.0f, "-24", juce::Colour(0xff64748b) },
            { -48.0f, "-inf",juce::Colour(0xff475569) }
        }};

        for (const auto& mark : marks)
        {
            float norm = (mark.db - (-60.0f)) / (6.0f - (-60.0f));
            norm = std::clamp(norm, 0.0f, 1.0f);
            float yPos = meterBottom - (norm * meterHeight);

            // Tick line extending from meter to scale
            g.setColour((mark.db == 0.0f) ? juce::Colour(0xff38bdf8) : juce::Colour(0xff334155));
            g.drawLine(scaleX, yPos, scaleX + 3.0f, yPos, (mark.db == 0.0f) ? 1.5f : 1.0f);

            // Text label
            g.setColour(mark.col);
            g.drawText(mark.label, static_cast<int>(scaleX + 4.0f), static_cast<int>(yPos - 5.0f), static_cast<int>(scaleW - 4.0f), 10, juce::Justification::centredLeft);
        }

        // Fader background track slot highlight
        const auto sliderBox = masterFaderSlider.getBounds().toFloat();
        g.setColour(juce::Colour(0xff090d16));
        g.fillRoundedRectangle(sliderBox.getCentreX() - 3.0f, meterTop, 6.0f, meterHeight, 3.0f);
        g.setColour(juce::Colour(0xff1e293b));
        g.drawRoundedRectangle(sliderBox.getCentreX() - 3.0f, meterTop, 6.0f, meterHeight, 3.0f, 0.8f);
    }
}

void MasterChannelStripComponent::resized()
{
    auto bounds = getLocalBounds().reduced(6);

    // Master Inserts Section at top
    masterInsertsTitleLabel.setBounds(bounds.removeFromTop(18));

    // Dynamic Viewport height based on number of master slots
    const int numSlots = static_cast<int>(graphManager.getMasterSlots().size());
    const int neededSlotsH = (numSlots * 70) + 4;
    // Leave at least 220px at bottom for Master Fader, VU meters and 0dB Reset
    const int availableForViewport = std::max(50, bounds.getHeight() - 230);
    const int maxViewportH = std::clamp(availableForViewport, 60, 240);
    const int viewportH = std::min(neededSlotsH, maxViewportH);
    masterSlotsViewport.setBounds(bounds.removeFromTop(viewportH));

    // Update width of all slot child components to match viewport
    const int viewW = masterSlotsViewport.getViewWidth();
    if (viewW > 20 && masterSlotsContainer != nullptr)
    {
        masterSlotsContainer->setSize(viewW, masterSlotsContainer->getHeight());
        for (auto& slotComp : masterSlotComponents)
        {
            if (slotComp != nullptr)
                slotComp->setBounds(0, slotComp->getY(), viewW, slotComp->getHeight());
        }
    }

    bounds.removeFromTop(4);

    // Add Master Slot button
    addMasterSlotButton.setBounds(bounds.removeFromTop(24).reduced(4, 1));

    bounds.removeFromTop(6); // separator

    // Master Fader Title
    titleLabel.setBounds(bounds.removeFromTop(18));

    bounds.removeFromTop(4);

    // Bottom Area for 0 dB Reset button and dB Readout
    auto bottomArea = bounds.removeFromBottom(54);
    reset0DbButton.setBounds(bottomArea.removeFromBottom(24).reduced(20, 1));
    bottomArea.removeFromBottom(2);
    faderDbLabel.setBounds(bottomArea.reduced(24, 0));

    // Remaining bounds is the Meter & Fader main area
    auto controlArea = bounds.reduced(4, 2);
    const int totalW = getWidth();

    // Group geometry: Meter (22px) + Gap (5px) + Scale (26px) + Gap (5px) + Fader (44px) = 102px total
    const int meterScaleW = 22 + 5 + 26 + 5; // 58px
    const int faderW = 44;
    const int groupW = meterScaleW + faderW; // 102px
    const int startX = std::max(6, (totalW - groupW) / 2);

    // Place Master Fader directly aligned with the centered group
    masterFaderSlider.setBounds(startX + meterScaleW, controlArea.getY(), faderW, controlArea.getHeight());
}

void MasterChannelStripComponent::timerCallback()
{
    const float l = graphManager.getLeftPeak();
    const float r = graphManager.getRightPeak();

    // Smooth meter animation
    currentLeftPeak = (l > currentLeftPeak) ? l : (currentLeftPeak * 0.82f);
    currentRightPeak = (r > currentRightPeak) ? r : (currentRightPeak * 0.82f);

    if (currentLeftPeak >= 1.0f || currentRightPeak >= 1.0f)
        isClipping = true;
    else if (currentLeftPeak < 0.9f && currentRightPeak < 0.9f)
        isClipping = false;

    repaint();
}
