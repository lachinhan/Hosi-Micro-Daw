#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include "../audio/AudioEngine.h"

class AudioSettingsOverlay : public juce::Component
{
public:
    explicit AudioSettingsOverlay(AudioEngine& engine);
    ~AudioSettingsOverlay() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    void updateScaleButtonsUI(float currentScale);

    std::function<void()> onCloseClicked;
    std::function<void(float scaleFactor, int targetW, int targetH)> onUiScaleChanged;

private:
    AudioEngine& audioEngine;

    juce::Label titleLabel;
    juce::TextButton closeButton{ "CLOSE" };

    // Window Resolution & UI Zoom Scale Presets
    juce::Label scaleTitleLabel;
    juce::TextButton btnScaleCompact{ "COMPACT (85% ZOOM)" };
    juce::TextButton btnScaleStandard{ "STANDARD (100% ZOOM)" };
    juce::TextButton btnScaleLarge{ "LARGE / 2K (125% ZOOM)" };

    std::unique_ptr<juce::AudioDeviceSelectorComponent> deviceSelector;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioSettingsOverlay)
};
