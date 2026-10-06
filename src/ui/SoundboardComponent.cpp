#include "SoundboardComponent.h"

// -----------------------------------------------------------------------------
// SoundPadButton Implementation
// -----------------------------------------------------------------------------

SoundPadButton::SoundPadButton(int padIdx, SoundboardAudioProcessor& proc)
    : juce::Button("Pad_" + juce::String(padIdx)), padIndex(padIdx), processor(proc)
{
    setTooltip(juce::String::fromUTF8(u8"Click trái: Phát / Dừng (Phím [") + juce::String(padIdx + 1) + juce::String::fromUTF8(u8"])\nClick đúp: Đổi tên nút\nClick phải: Menu tùy biến (Đổi tên, Nạp file, Reset)"));
}

void SoundPadButton::clicked()
{
    if (processor.isPadPlaying(padIndex))
    {
        processor.stopPad(padIndex);
    }
    else
    {
        processor.triggerPad(padIndex);
    }
}

void SoundPadButton::clicked(const juce::ModifierKeys& modifiers)
{
    if (modifiers.isPopupMenu() || modifiers.isRightButtonDown())
    {
        showContextMenu();
    }
    else
    {
        clicked();
    }
}

void SoundPadButton::mouseDoubleClick(const juce::MouseEvent& /*e*/)
{
    showRenameDialog();
}

void SoundPadButton::showContextMenu()
{
    juce::PopupMenu menu;
    menu.addItem(1, juce::String::fromUTF8(u8"✏  Đổi tên nút (Rename)..."));
    menu.addItem(2, juce::String::fromUTF8(u8"📁  Nạp file âm thanh (.wav, .mp3)..."));
    menu.addSeparator();
    menu.addItem(3, juce::String::fromUTF8(u8"↺  Khôi phục mặc định (Reset)"));

    menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(this), [this](int result) {
        if (result == 1)
        {
            showRenameDialog();
        }
        else if (result == 2)
        {
            loadCustomSample();
        }
        else if (result == 3)
        {
            resetToDefault();
        }
    });
}

void SoundPadButton::showRenameDialog()
{
    auto* alert = new juce::AlertWindow(
        juce::String::fromUTF8(u8"Đổi Tên Nút Pad [") + juce::String(padIndex + 1) + "]",
        juce::String::fromUTF8(u8"Nhập tên mới cho nút Soundboard:"),
        juce::AlertWindow::QuestionIcon
    );

    const juce::String currentName = processor.getPadData(padIndex).name;
    alert->addTextEditor("padName", currentName, juce::String::fromUTF8(u8"Tên nút..."));
    alert->addButton(juce::String::fromUTF8(u8"Lưu Tên"), 1, juce::KeyPress(juce::KeyPress::returnKey));
    alert->addButton(juce::String::fromUTF8(u8"Hủy"), 0, juce::KeyPress(juce::KeyPress::escapeKey));

    alert->enterModalState(true, juce::ModalCallbackFunction::create([this, alert](int result) {
        if (result == 1)
        {
            auto newName = alert->getTextEditorContents("padName").trim();
            if (newName.isNotEmpty())
            {
                processor.setPadName(padIndex, newName);
                repaint();
            }
        }
        delete alert;
    }), true);
}

void SoundPadButton::resetToDefault()
{
    processor.resetPad(padIndex);
    repaint();
}

void SoundPadButton::loadCustomSample()
{
    fileChooser = std::make_unique<juce::FileChooser>(
        juce::String::fromUTF8(u8"Chọn File Âm Thanh Cho Pad [") + juce::String(padIndex + 1) + "]...",
        juce::File::getSpecialLocation(juce::File::userMusicDirectory),
        "*.wav;*.mp3;*.flac;*.ogg;*.aiff"
    );

    auto flags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles;
    fileChooser->launchAsync(flags, [this](const juce::FileChooser& fc) {
        auto result = fc.getResult();
        if (result.existsAsFile())
        {
            juce::String err;
            if (!processor.loadCustomSample(padIndex, result, err))
            {
                juce::AlertWindow::showMessageBoxAsync(
                    juce::AlertWindow::WarningIcon,
                    juce::String::fromUTF8(u8"Lỗi đọc file âm thanh"),
                    err,
                    "OK"
                );
            }
            else
            {
                // Auto-suggest pad name from file name if user hasn't custom renamed it
                const juce::String currentName = processor.getPadData(padIndex).name;
                const juce::String fileNameWithoutExt = result.getFileNameWithoutExtension();
                if (currentName.startsWith("Pad ") || currentName.isEmpty())
                {
                    processor.setPadName(padIndex, fileNameWithoutExt.toUpperCase());
                }
                repaint();
            }
        }
    });
}

void SoundPadButton::paintButton(juce::Graphics& g, bool isMouseOverButton, bool isButtonDown)
{
    auto bounds = getLocalBounds().toFloat().reduced(2.0f);
    const auto& padData = processor.getPadData(padIndex);
    const bool isPlaying = processor.isPadPlaying(padIndex);

    juce::Colour baseColour = padData.padColour;
    if (isButtonDown)
        baseColour = baseColour.brighter(0.4f);
    else if (isMouseOverButton)
        baseColour = baseColour.brighter(0.2f);

    // Glowing background when playing
    if (isPlaying)
    {
        g.setColour(baseColour.withAlpha(0.35f));
        g.fillRoundedRectangle(bounds.expanded(2.0f), 6.0f);

        g.setColour(baseColour);
        g.fillRoundedRectangle(bounds, 5.0f);

        g.setColour(juce::Colours::white);
        g.drawRoundedRectangle(bounds, 5.0f, 2.0f);
    }
    else
    {
        g.setColour(juce::Colour(0xff161e2e));
        g.fillRoundedRectangle(bounds, 5.0f);

        // Accent top bar
        g.setColour(baseColour.withAlpha(0.65f));
        g.fillRoundedRectangle(bounds.getX(), bounds.getY(), bounds.getWidth(), 4.0f, 2.0f);

        g.setColour(juce::Colour(0xff222f44));
        g.drawRoundedRectangle(bounds, 5.0f, 1.0f);
    }

    // Number Shortcut badge in top-left
    auto topRow = bounds.removeFromTop(16.0f);
    auto badgeArea = topRow.removeFromLeft(24.0f).reduced(2.0f, 1.0f);
    g.setColour(isPlaying ? juce::Colours::white : baseColour);
    g.setFont(juce::FontOptions(10.0f, juce::Font::bold));
    g.drawText("[" + juce::String(padIndex + 1) + "]", badgeArea, juce::Justification::centredLeft);

    // Edit indicator in top-right when mouse hover
    if (isMouseOverButton && !isPlaying)
    {
        auto editHintArea = topRow.removeFromRight(20.0f).reduced(2.0f, 1.0f);
        g.setColour(juce::Colour(0xff94a3b8));
        g.setFont(juce::FontOptions(9.5f, juce::Font::plain));
        g.drawText(juce::String::fromUTF8(u8"✎"), editHintArea, juce::Justification::centredRight);
    }

    // Pad Name in center
    g.setColour(isPlaying ? juce::Colours::white : juce::Colour(0xffe2e8f0));
    g.setFont(juce::FontOptions(11.0f, juce::Font::bold));
    g.drawFittedText(padData.name, bounds.toNearestInt().reduced(4, 2), juce::Justification::centred, 2);
}

// -----------------------------------------------------------------------------
// SoundboardComponent Implementation
// -----------------------------------------------------------------------------

SoundboardComponent::SoundboardComponent(GraphManager& graphMgr)
    : graphManager(graphMgr)
{
    soundboardProcessor = graphManager.getSoundboard();
    if (soundboardProcessor != nullptr)
    {
        soundboardProcessor->addChangeListener(this);
    }

    // Title Label
    titleLabel.setText(juce::String::fromUTF8(u8"SOUNDBOARD LIVE"), juce::dontSendNotification);
    titleLabel.setFont(juce::FontOptions(12.0f, juce::Font::bold));
    titleLabel.setColour(juce::Label::textColourId, juce::Colour(0xfff59e0b)); // Gold
    titleLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(titleLabel);

    // Stop All Button
    stopAllButton.setButtonText("STOP ALL (ESC)");
    stopAllButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff7f1d1d)); // Dark Red
    stopAllButton.setColour(juce::TextButton::textColourOffId, juce::Colour(0xfffca5a5));
    stopAllButton.onClick = [this] { stopAll(); };
    addAndMakeVisible(stopAllButton);

    // Volume Slider
    volumeSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    volumeSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    volumeSlider.setRange(0.0, 1.5, 0.01);
    volumeSlider.setValue(1.0);
    volumeSlider.setColour(juce::Slider::trackColourId, juce::Colour(0xfff59e0b));
    volumeSlider.setColour(juce::Slider::backgroundColourId, juce::Colour(0xff1e293b));
    volumeSlider.setColour(juce::Slider::thumbColourId, juce::Colour(0xfffbbf24));
    volumeSlider.onValueChange = [this] {
        if (soundboardProcessor != nullptr)
        {
            soundboardProcessor->setMasterSoundboardGain(static_cast<float>(volumeSlider.getValue()));
        }
    };
    addAndMakeVisible(volumeSlider);

    volumeLabel.setText("VOL", juce::dontSendNotification);
    volumeLabel.setFont(juce::FontOptions(10.0f, juce::Font::bold));
    volumeLabel.setColour(juce::Label::textColourId, juce::Colour(0xff64748b));
    volumeLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(volumeLabel);

    // Create 8 Sound Pad Buttons
    if (soundboardProcessor != nullptr)
    {
        for (int i = 0; i < SoundboardAudioProcessor::NUM_PADS; ++i)
        {
            auto btn = std::make_unique<SoundPadButton>(i, *soundboardProcessor);
            addAndMakeVisible(btn.get());
            padButtons.push_back(std::move(btn));
        }
    }

    startTimerHz(25); // 25 FPS UI refresh
}

SoundboardComponent::~SoundboardComponent()
{
    stopTimer();
    if (soundboardProcessor != nullptr)
    {
        soundboardProcessor->removeChangeListener(this);
    }
}

void SoundboardComponent::triggerPad(int padIndex)
{
    if (soundboardProcessor != nullptr)
    {
        soundboardProcessor->triggerPad(padIndex);
        repaint();
    }
}

void SoundboardComponent::stopAll()
{
    if (soundboardProcessor != nullptr)
    {
        soundboardProcessor->stopAll();
        repaint();
    }
}

void SoundboardComponent::timerCallback()
{
    repaint();
}

void SoundboardComponent::changeListenerCallback(juce::ChangeBroadcaster* /*source*/)
{
    repaint();
}

void SoundboardComponent::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    // Dark glassmorphism background
    g.setColour(juce::Colour(0xff0d111a));
    g.fillRoundedRectangle(bounds, 6.0f);

    g.setColour(juce::Colour(0xff1e293b));
    g.drawRoundedRectangle(bounds, 6.0f, 1.0f);
}

void SoundboardComponent::resized()
{
    auto area = getLocalBounds().reduced(6);

    // Top Header: Title
    titleLabel.setBounds(area.removeFromTop(20));

    // Volume Row
    auto volRow = area.removeFromTop(24);
    volumeLabel.setBounds(volRow.removeFromLeft(28));
    volumeSlider.setBounds(volRow.reduced(2, 4));

    area.removeFromTop(4);

    // Stop All Button
    stopAllButton.setBounds(area.removeFromTop(24).reduced(2, 0));

    area.removeFromTop(8);

    // 8 Pads arranged in 2 columns x 4 rows
    const int numCols = 2;
    const int numRows = 4;
    const int padSpacing = 4;

    const int padW = (area.getWidth() - padSpacing) / numCols;
    const int padH = (area.getHeight() - (numRows - 1) * padSpacing) / numRows;

    for (int i = 0; i < static_cast<int>(padButtons.size()); ++i)
    {
        const int col = i % numCols;
        const int row = i / numCols;

        const int x = area.getX() + col * (padW + padSpacing);
        const int y = area.getY() + row * (padH + padSpacing);

        padButtons[static_cast<size_t>(i)]->setBounds(x, y, padW, padH);
    }
}
