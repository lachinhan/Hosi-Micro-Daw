#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"

class OBSReceiverAudioProcessorEditor : public juce::AudioProcessorEditor, public juce::Timer
{
public:
    explicit OBSReceiverAudioProcessorEditor(OBSReceiverAudioProcessor&);
    ~OBSReceiverAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;

private:
    OBSReceiverAudioProcessor& audioProcessor;

    juce::Label titleLabel;
    juce::Label statusLabel;
    float currentMeterLevel{ 0.0f };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OBSReceiverAudioProcessorEditor)
};
