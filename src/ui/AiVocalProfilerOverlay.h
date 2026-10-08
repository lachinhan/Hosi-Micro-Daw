#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../audio/BuiltInDspAudioProcessor.h"

class AiVocalProfilerOverlay : public juce::Component, public juce::Timer
{
public:
    AiVocalProfilerOverlay(BuiltInDspAudioProcessor& dsp);
    ~AiVocalProfilerOverlay() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void timerCallback() override;

    std::function<void()> onClose;

private:
    BuiltInDspAudioProcessor& dspProcessor;

    juce::Label titleLabel;
    juce::TextButton closeButton{ "✕" };

    juce::Label instructionLabel;
    juce::TextButton startRecordButton;
    juce::ProgressBar progressBar;
    double progressVal{ 0.0 };

    juce::GroupComponent resultsGroup;
    juce::Label voiceTypeLabel;
    juce::Label diagnosticsLabel;

    juce::Label styleTitleLabel;
    juce::ComboBox styleComboBox;

    juce::Label eqPreviewLabel;
    juce::TextButton applyButton;

    AiVocalProfiler::ProfileStyle currentStyle{ AiVocalProfiler::ProfileStyle::StudioMaster };

    void handleStartRecord();
    void updateResultsUI();
    void updateEqPreview();
    void handleApplyEq();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AiVocalProfilerOverlay)
};
