#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../audio/BuiltInDspAudioProcessor.h"
#include "../audio/GraphManager.h"
#include "../audio/TempoSyncEngine.h"
#include "AiVocalProfilerOverlay.h"

class BuiltInDspComponent : public juce::Component, public juce::Timer, public juce::ChangeListener
{
public:
    explicit BuiltInDspComponent(GraphManager& graphMgr);
    ~BuiltInDspComponent() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void timerCallback() override;
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;

    void updateAllUI();
    void showAiVocalProfilerOverlay();

private:
    GraphManager& graphManager;
    BuiltInDspAudioProcessor* dspProcessor{ nullptr };

    // Viewport to allow smooth scrolling if window is resized vertically
    juce::Viewport viewport;
    std::unique_ptr<juce::Component> contentContainer;
    std::unique_ptr<AiVocalProfilerOverlay> profilerOverlay;

    // Header & Presets
    juce::Label headerTitleLabel;
    juce::ComboBox presetComboBox;
    juce::TextButton factoryResetButton{ "RESET" };

    // Global Tempo (BPM) Bar
    juce::Label bpmTitleLabel;
    juce::Label bpmValueLabel;
    juce::TextButton bpmDownBtn{ "-" };
    juce::TextButton bpmUpBtn{ "+" };
    juce::TextButton tapTempoBtn{ "TAP" };

    // --- 1. AI Noise & Room De-Reverb Shield ---
    juce::TextButton aiPwrButton{ "PWR" };
    juce::Label aiTitleLabel;
    juce::TextButton aiDenoiseToggle{ "⚡ AI DENOISE" };
    juce::TextButton aiDeReverbToggle{ "🏠 DE-REVERB" };
    juce::Slider aiDenoiseSlider;
    juce::Label aiDenoiseLabel;
    juce::Slider aiDeReverbSlider;
    juce::Label aiDeReverbLabel;
    juce::Label aiStatusLabel;

    // --- 2. Noise Gate Module ---
    juce::TextButton gatePwrButton{ "PWR" };
    juce::Label gateTitleLabel;
    juce::Slider gateThreshSlider;
    juce::Label gateThreshLabel;
    bool gateIsOpenCached{ false };

    // --- 3. Studio EQ Module ---
    juce::TextButton eqPwrButton{ "PWR" };
    juce::Label eqTitleLabel;
    juce::TextButton aiAutoEqButton{ "✨ AI AUTO-EQ" };
    juce::Slider eqLowSlider, eqMidSlider, eqHighSlider;
    juce::Label eqLowLabel, eqMidLabel, eqHighLabel;

    // --- 3. Warm Comp Module ---
    juce::TextButton compPwrButton{ "PWR" };
    juce::Label compTitleLabel;
    juce::Slider compThreshSlider, compRatioSlider, compMakeupSlider;
    juce::Label compThreshLabel, compRatioLabel, compMakeupLabel;
    float compGrCached{ 0.0f };

    // --- 4. Lush Reverb Module & Smart Auto-Tail ---
    juce::TextButton reverbPwrButton{ "PWR" };
    juce::Label reverbTitleLabel;
    juce::TextButton reverbSyncToggle{ "⚡ AUTO-TAIL" };
    juce::ComboBox reverbBarCombo;
    juce::Label reverbDecayInfoLabel;
    juce::Slider reverbSizeSlider, reverbDampSlider, reverbWetSlider;
    juce::Label reverbSizeLabel, reverbDampLabel, reverbWetLabel;

    // --- 5. Stereo Delay Module & Smart BPM Sync ---
    juce::TextButton delayPwrButton{ "PWR" };
    juce::Label delayTitleLabel;
    juce::TextButton delaySyncToggle{ "⚡ BPM SYNC" };
    juce::ComboBox delaySubdivisionCombo;
    juce::Label delayTimeInfoLabel;
    juce::Slider delayTimeSlider, delayFeedbackSlider, delayWetSlider;
    juce::Label delayTimeLabel, delayFeedbackLabel, delayWetLabel;

    // --- 6. Brickwall Limiter Module ---
    juce::TextButton limiterPwrButton{ "PWR" };
    juce::Label limiterTitleLabel;
    juce::Slider limiterThreshSlider;
    juce::Label limiterThreshLabel;

    void setupModuleHeader(juce::TextButton& pwrBtn, juce::Label& titleLbl, const juce::String& titleText);
    void setupSlider(juce::Slider& slider, juce::Label& label, const juce::String& name, double min, double max, double step, double def, const juce::String& suffix);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BuiltInDspComponent)
};
