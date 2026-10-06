#include "AudioSettingsOverlay.h"

AudioSettingsOverlay::AudioSettingsOverlay(AudioEngine& engine)
    : audioEngine(engine)
{
    titleLabel.setText("SETTINGS & HARDWARE CONFIGURATION", juce::dontSendNotification);
    titleLabel.setFont(juce::FontOptions(17.0f, juce::Font::bold));
    titleLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    titleLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(titleLabel);

    closeButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff2a2e39));
    closeButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    closeButton.onClick = [this]() {
        if (onCloseClicked)
            onCloseClicked();
    };
    addAndMakeVisible(closeButton);

    // Window Resolution & UI Zoom Scale Section
    scaleTitleLabel.setText("RESOLUTION & UI SCALE PRESETS:", juce::dontSendNotification);
    scaleTitleLabel.setFont(juce::FontOptions(12.0f, juce::Font::bold));
    scaleTitleLabel.setColour(juce::Label::textColourId, juce::Colour(0xfff59e0b)); // Amber Gold
    addAndMakeVisible(scaleTitleLabel);

    btnScaleCompact.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff1e293b));
    btnScaleCompact.setColour(juce::TextButton::textColourOffId, juce::Colour(0xff94a3b8));
    btnScaleCompact.setTooltip(juce::String::fromUTF8(u8"Thu nhỏ tỷ lệ 85% - Toàn bộ giao diện thu nhỏ vừa vặn màn hình Laptop"));
    btnScaleCompact.onClick = [this]() {
        if (onUiScaleChanged)
            onUiScaleChanged(0.85f, 1140, 760);
        updateScaleButtonsUI(0.85f);
    };
    addAndMakeVisible(btnScaleCompact);

    btnScaleStandard.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff0284c7)); // Sky Blue Active
    btnScaleStandard.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    btnScaleStandard.setTooltip(juce::String::fromUTF8(u8"Tỷ lệ chuẩn 100% - Kích thước mặc định cân bằng và sắc nét"));
    btnScaleStandard.onClick = [this]() {
        if (onUiScaleChanged)
            onUiScaleChanged(1.00f, 1140, 760);
        updateScaleButtonsUI(1.00f);
    };
    addAndMakeVisible(btnScaleStandard);

    btnScaleLarge.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff1e293b));
    btnScaleLarge.setColour(juce::TextButton::textColourOffId, juce::Colour(0xff94a3b8));
    btnScaleLarge.setTooltip(juce::String::fromUTF8(u8"Phóng to tỷ lệ 125% - Chữ và nút siêu to rõ, dễ nhìn cho mắt yếu / người lớn tuổi / màn 2K 4K"));
    btnScaleLarge.onClick = [this]() {
        if (onUiScaleChanged)
            onUiScaleChanged(1.25f, 1140, 760);
        updateScaleButtonsUI(1.25f);
    };
    addAndMakeVisible(btnScaleLarge);

    // juce::AudioDeviceSelectorComponent config (support multi-channel interfaces like QUAD-CAPTURE)
    deviceSelector = std::make_unique<juce::AudioDeviceSelectorComponent>(
        audioEngine.getDeviceManager(),
        1, 8, // min/max input channels (supports multi-channel ASIO interfaces)
        1, 8, // min/max output channels
        false, // show midi inputs
        false, // show midi output
        false, // show channels as stereo pairs
        false  // hide advanced options
    );
    addAndMakeVisible(deviceSelector.get());
}

AudioSettingsOverlay::~AudioSettingsOverlay()
{
}

void AudioSettingsOverlay::updateScaleButtonsUI(float currentScale)
{
    // Reset all buttons to default dark slate
    btnScaleCompact.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff1e293b));
    btnScaleCompact.setColour(juce::TextButton::textColourOffId, juce::Colour(0xff94a3b8));

    btnScaleStandard.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff1e293b));
    btnScaleStandard.setColour(juce::TextButton::textColourOffId, juce::Colour(0xff94a3b8));

    btnScaleLarge.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff1e293b));
    btnScaleLarge.setColour(juce::TextButton::textColourOffId, juce::Colour(0xff94a3b8));

    if (currentScale <= 0.90f)
    {
        btnScaleCompact.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff059669)); // Green
        btnScaleCompact.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    }
    else if (currentScale < 1.15f)
    {
        btnScaleStandard.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff0284c7)); // Sky Blue
        btnScaleStandard.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    }
    else
    {
        btnScaleLarge.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff9333ea)); // Purple
        btnScaleLarge.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    }
    repaint();
}

void AudioSettingsOverlay::paint(juce::Graphics& g)
{
    // Semi-transparent backdrop
    g.fillAll(juce::Colour(0xd00a0d14));

    // Center dialog card
    auto bounds = getLocalBounds().reduced(30);
    g.setColour(juce::Colour(0xff181b24));
    g.fillRoundedRectangle(bounds.toFloat(), 12.0f);

    // Border highlight
    g.setColour(juce::Colour(0xff363c4e));
    g.drawRoundedRectangle(bounds.toFloat(), 12.0f, 1.5f);
}

void AudioSettingsOverlay::resized()
{
    auto bounds = getLocalBounds().reduced(40);

    auto headerArea = bounds.removeFromTop(40);
    closeButton.setBounds(headerArea.removeFromRight(80).reduced(4));
    titleLabel.setBounds(headerArea);

    bounds.removeFromTop(10);

    // Scale Presets Bar
    auto scaleArea = bounds.removeFromTop(32);
    scaleTitleLabel.setBounds(scaleArea.removeFromLeft(200));
    btnScaleCompact.setBounds(scaleArea.removeFromLeft(170).reduced(3, 1));
    btnScaleStandard.setBounds(scaleArea.removeFromLeft(170).reduced(3, 1));
    btnScaleLarge.setBounds(scaleArea.removeFromLeft(170).reduced(3, 1));

    bounds.removeFromTop(12);

    if (deviceSelector != nullptr)
    {
        deviceSelector->setBounds(bounds);
    }
}
