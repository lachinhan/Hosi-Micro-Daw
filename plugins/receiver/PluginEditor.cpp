#include "PluginProcessor.h"
#include "PluginEditor.h"

OBSReceiverAudioProcessorEditor::OBSReceiverAudioProcessorEditor(OBSReceiverAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    setSize(360, 160);

    titleLabel.setText("OBS IPC RECEIVER", juce::dontSendNotification);
    titleLabel.setFont(juce::FontOptions(15.0f, juce::Font::bold));
    titleLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    titleLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(titleLabel);

    statusLabel.setFont(juce::FontOptions(12.0f, juce::Font::plain));
    statusLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(statusLabel);

    startTimerHz(30);
}

OBSReceiverAudioProcessorEditor::~OBSReceiverAudioProcessorEditor()
{
    stopTimer();
}

void OBSReceiverAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff0f1118));

    auto bounds = getLocalBounds().toFloat().reduced(8.0f);
    g.setColour(juce::Colour(0xff1f2433));
    g.drawRoundedRectangle(bounds, 8.0f, 1.5f);

    // Audio VU Level Meter
    auto meterBounds = juce::Rectangle<float>(24.0f, 95.0f, static_cast<float>(getWidth() - 48), 16.0f);
    g.setColour(juce::Colour(0xff171a24));
    g.fillRoundedRectangle(meterBounds, 4.0f);

    const float fillWidth = std::clamp(currentMeterLevel * 3.0f, 0.0f, 1.0f) * meterBounds.getWidth();
    if (fillWidth > 0.0f)
    {
        auto fillRect = meterBounds.withWidth(fillWidth);
        g.setColour(juce::Colour(0xff00e676));
        g.fillRoundedRectangle(fillRect, 4.0f);
    }
}

void OBSReceiverAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds().reduced(12);
    titleLabel.setBounds(bounds.removeFromTop(28));
    statusLabel.setBounds(bounds.removeFromTop(24));
}

void OBSReceiverAudioProcessorEditor::timerCallback()
{
    const bool connected = audioProcessor.isConnectedToDaw();
    if (connected)
    {
        statusLabel.setText("CONNECTED TO MICRO-DAW (Active)", juce::dontSendNotification);
        statusLabel.setColour(juce::Label::textColourId, juce::Colour(0xff00e676));
    }
    else
    {
        statusLabel.setText("WAITING FOR MICRO-DAW STREAM...", juce::dontSendNotification);
        statusLabel.setColour(juce::Label::textColourId, juce::Colour(0xffff9100));
    }

    currentMeterLevel = audioProcessor.getRmsLevel();
    repaint();
}
