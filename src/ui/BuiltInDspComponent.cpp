#include "BuiltInDspComponent.h"

BuiltInDspComponent::BuiltInDspComponent(GraphManager& graphMgr)
    : graphManager(graphMgr)
{
    dspProcessor = graphManager.getBuiltInDsp();
    if (dspProcessor != nullptr)
        dspProcessor->addChangeListener(this);

    contentContainer = std::make_unique<juce::Component>();

    // Header Title
    headerTitleLabel.setText(juce::String::fromUTF8(u8"STUDIO VOCAL DSP SUITE"), juce::dontSendNotification);
    headerTitleLabel.setFont(juce::FontOptions(11.0f, juce::Font::bold));
    headerTitleLabel.setColour(juce::Label::textColourId, juce::Colour(0xff38bdf8));
    headerTitleLabel.setJustificationType(juce::Justification::centred);
    contentContainer->addAndMakeVisible(headerTitleLabel);

    // Preset Combo Box
    presetComboBox.addItem(juce::String::fromUTF8(u8"✨ 1. Hát Live (Warm & Lush)"), 1);
    presetComboBox.addItem(juce::String::fromUTF8(u8"🎙️ 2. Streamer / MC Talk"), 2);
    presetComboBox.addItem(juce::String::fromUTF8(u8"🎤 3. Karaoke Hall Echo"), 3);
    presetComboBox.addItem(juce::String::fromUTF8(u8"🎧 4. Podcast Clean Vocal"), 4);
    presetComboBox.addItem(juce::String::fromUTF8(u8"⚡ 5. Tắt DSP (Bypass All)"), 5);
    presetComboBox.addItem(juce::String::fromUTF8(u8"🔄 6. Khôi Phục Mặc Định (Reset)"), 6);
    presetComboBox.setSelectedId(1, juce::dontSendNotification);
    presetComboBox.setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xff1e293b));
    presetComboBox.setColour(juce::ComboBox::textColourId, juce::Colour(0xfff8fafc));
    presetComboBox.onChange = [this] {
        if (dspProcessor != nullptr)
        {
            int id = presetComboBox.getSelectedId();
            if (id == 6)
            {
                dspProcessor->resetToFactoryDefaults();
                updateAllUI();
            }
            else
            {
                dspProcessor->loadPreset(static_cast<BuiltInDspAudioProcessor::VocalPreset>(id - 1));
                updateAllUI();
            }
        }
    };
    contentContainer->addAndMakeVisible(presetComboBox);

    // Factory Reset Button
    factoryResetButton.setButtonText(juce::String::fromUTF8(u8"🔄 KHÔI PHỤC GỐC"));
    factoryResetButton.setTooltip(juce::String::fromUTF8(u8"Khôi phục toàn bộ thông số Vocal DSP về chuẩn Studio ban đầu"));
    factoryResetButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff1e293b));
    factoryResetButton.setColour(juce::TextButton::textColourOffId, juce::Colour(0xfff87171));
    factoryResetButton.onClick = [this] {
        if (dspProcessor != nullptr)
        {
            dspProcessor->resetToFactoryDefaults();
            updateAllUI();
        }
    };
    contentContainer->addAndMakeVisible(factoryResetButton);

    // --- 1. Noise Gate ---
    setupModuleHeader(gatePwrButton, gateTitleLabel, juce::String::fromUTF8(u8"1. NOISE GATE (CHỐNG ỒN)"));
    gatePwrButton.onClick = [this] {
        if (dspProcessor != nullptr) {
            dspProcessor->setGateEnabled(!dspProcessor->isGateEnabled());
            updateAllUI();
        }
    };
    setupSlider(gateThreshSlider, gateThreshLabel, "Thresh", -80.0, 0.0, 0.5, -48.0, " dB");
    gateThreshSlider.onValueChange = [this] {
        if (dspProcessor != nullptr) dspProcessor->setGateThresholdDb(static_cast<float>(gateThreshSlider.getValue()));
    };

    // --- 2. Studio EQ ---
    setupModuleHeader(eqPwrButton, eqTitleLabel, juce::String::fromUTF8(u8"2. STUDIO EQ 3-BAND"));
    eqPwrButton.onClick = [this] {
        if (dspProcessor != nullptr) {
            dspProcessor->setEqEnabled(!dspProcessor->isEqEnabled());
            updateAllUI();
        }
    };
    setupSlider(eqLowSlider, eqLowLabel, "Low (120Hz)", -12.0, 12.0, 0.5, 0.0, " dB");
    eqLowSlider.onValueChange = [this] {
        if (dspProcessor != nullptr) dspProcessor->setEqLowGainDb(static_cast<float>(eqLowSlider.getValue()));
    };
    setupSlider(eqMidSlider, eqMidLabel, "Mid (2.6k)", -12.0, 12.0, 0.5, 2.0, " dB");
    eqMidSlider.onValueChange = [this] {
        if (dspProcessor != nullptr) dspProcessor->setEqMidGainDb(static_cast<float>(eqMidSlider.getValue()));
    };
    setupSlider(eqHighSlider, eqHighLabel, "Air (9.5k)", -12.0, 12.0, 0.5, 2.5, " dB");
    eqHighSlider.onValueChange = [this] {
        if (dspProcessor != nullptr) dspProcessor->setEqHighGainDb(static_cast<float>(eqHighSlider.getValue()));
    };

    // --- 3. Warm Comp ---
    setupModuleHeader(compPwrButton, compTitleLabel, juce::String::fromUTF8(u8"3. WARM COMPRESSOR"));
    compPwrButton.onClick = [this] {
        if (dspProcessor != nullptr) {
            dspProcessor->setCompEnabled(!dspProcessor->isCompEnabled());
            updateAllUI();
        }
    };
    setupSlider(compThreshSlider, compThreshLabel, "Thresh", -40.0, 0.0, 0.5, -18.0, " dB");
    compThreshSlider.onValueChange = [this] {
        if (dspProcessor != nullptr) dspProcessor->setCompThresholdDb(static_cast<float>(compThreshSlider.getValue()));
    };
    setupSlider(compRatioSlider, compRatioLabel, "Ratio", 1.0, 8.0, 0.1, 3.2, ":1");
    compRatioSlider.onValueChange = [this] {
        if (dspProcessor != nullptr) dspProcessor->setCompRatio(static_cast<float>(compRatioSlider.getValue()));
    };
    setupSlider(compMakeupSlider, compMakeupLabel, "Makeup", 0.0, 12.0, 0.5, 2.5, " dB");
    compMakeupSlider.onValueChange = [this] {
        if (dspProcessor != nullptr) dspProcessor->setCompMakeupDb(static_cast<float>(compMakeupSlider.getValue()));
    };

    // --- 4. Lush Reverb ---
    setupModuleHeader(reverbPwrButton, reverbTitleLabel, juce::String::fromUTF8(u8"4. LUSH REVERB (KHÔNG GIAN)"));
    reverbPwrButton.onClick = [this] {
        if (dspProcessor != nullptr) {
            dspProcessor->setReverbEnabled(!dspProcessor->isReverbEnabled());
            updateAllUI();
        }
    };
    setupSlider(reverbSizeSlider, reverbSizeLabel, "Size", 0.0, 100.0, 1.0, 65.0, "%");
    reverbSizeSlider.onValueChange = [this] {
        if (dspProcessor != nullptr) dspProcessor->setReverbSize(static_cast<float>(reverbSizeSlider.getValue() * 0.01));
    };
    setupSlider(reverbDampSlider, reverbDampLabel, "Damping", 0.0, 100.0, 1.0, 35.0, "%");
    reverbDampSlider.onValueChange = [this] {
        if (dspProcessor != nullptr) dspProcessor->setReverbDamp(static_cast<float>(reverbDampSlider.getValue() * 0.01));
    };
    setupSlider(reverbWetSlider, reverbWetLabel, "Wet Mix", 0.0, 100.0, 1.0, 22.0, "%");
    reverbWetSlider.onValueChange = [this] {
        if (dspProcessor != nullptr) dspProcessor->setReverbWetMix(static_cast<float>(reverbWetSlider.getValue() * 0.01));
    };

    // --- 5. Stereo Delay ---
    setupModuleHeader(delayPwrButton, delayTitleLabel, juce::String::fromUTF8(u8"5. STEREO DELAY / ECHO"));
    delayPwrButton.onClick = [this] {
        if (dspProcessor != nullptr) {
            dspProcessor->setDelayEnabled(!dspProcessor->isDelayEnabled());
            updateAllUI();
        }
    };
    setupSlider(delayTimeSlider, delayTimeLabel, "Time", 40.0, 800.0, 5.0, 260.0, " ms");
    delayTimeSlider.onValueChange = [this] {
        if (dspProcessor != nullptr) dspProcessor->setDelayTimeMs(static_cast<float>(delayTimeSlider.getValue()));
    };
    setupSlider(delayFeedbackSlider, delayFeedbackLabel, "Feedback", 0.0, 80.0, 1.0, 25.0, "%");
    delayFeedbackSlider.onValueChange = [this] {
        if (dspProcessor != nullptr) dspProcessor->setDelayFeedback(static_cast<float>(delayFeedbackSlider.getValue() * 0.01));
    };
    setupSlider(delayWetSlider, delayWetLabel, "Wet Mix", 0.0, 100.0, 1.0, 18.0, "%");
    delayWetSlider.onValueChange = [this] {
        if (dspProcessor != nullptr) dspProcessor->setDelayWetMix(static_cast<float>(delayWetSlider.getValue() * 0.01));
    };

    // --- 6. Brickwall Limiter ---
    setupModuleHeader(limiterPwrButton, limiterTitleLabel, juce::String::fromUTF8(u8"6. BRICKWALL LIMITER"));
    limiterPwrButton.onClick = [this] {
        if (dspProcessor != nullptr) {
            dspProcessor->setLimiterEnabled(!dspProcessor->isLimiterEnabled());
            updateAllUI();
        }
    };
    setupSlider(limiterThreshSlider, limiterThreshLabel, "Ceiling", -6.0, 0.0, 0.1, -0.5, " dB");
    limiterThreshSlider.onValueChange = [this] {
        if (dspProcessor != nullptr) dspProcessor->setLimiterThresholdDb(static_cast<float>(limiterThreshSlider.getValue()));
    };

    // Setup viewport
    viewport.setViewedComponent(contentContainer.get(), false);
    viewport.setScrollBarsShown(true, false, true, false);
    viewport.getVerticalScrollBar().setColour(juce::ScrollBar::thumbColourId, juce::Colour(0xff334155));
    addAndMakeVisible(viewport);

    updateAllUI();
    startTimerHz(25);
}

BuiltInDspComponent::~BuiltInDspComponent()
{
    stopTimer();
    if (dspProcessor != nullptr)
        dspProcessor->removeChangeListener(this);
}

void BuiltInDspComponent::setupModuleHeader(juce::TextButton& pwrBtn, juce::Label& titleLbl, const juce::String& titleText)
{
    pwrBtn.setButtonText("ON");
    pwrBtn.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff059669));
    pwrBtn.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    contentContainer->addAndMakeVisible(pwrBtn);

    titleLbl.setText(titleText, juce::dontSendNotification);
    titleLbl.setFont(juce::FontOptions(10.0f, juce::Font::bold));
    titleLbl.setColour(juce::Label::textColourId, juce::Colour(0xffe2e8f0));
    titleLbl.setJustificationType(juce::Justification::centredLeft);
    contentContainer->addAndMakeVisible(titleLbl);
}

void BuiltInDspComponent::setupSlider(juce::Slider& slider, juce::Label& label, const juce::String& name, double min, double max, double step, double def, const juce::String& suffix)
{
    slider.setSliderStyle(juce::Slider::LinearHorizontal);
    slider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 52, 16);
    slider.setRange(min, max, step);
    slider.setValue(def, juce::dontSendNotification);
    slider.setTextValueSuffix(suffix);
    slider.setColour(juce::Slider::trackColourId, juce::Colour(0xff0284c7));
    slider.setColour(juce::Slider::backgroundColourId, juce::Colour(0xff0f172a));
    slider.setColour(juce::Slider::thumbColourId, juce::Colour(0xff38bdf8));
    slider.setColour(juce::Slider::textBoxTextColourId, juce::Colour(0xfff8fafc));
    slider.setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour(0xff1e293b));
    slider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colour(0xff334155));
    contentContainer->addAndMakeVisible(slider);

    label.setText(name, juce::dontSendNotification);
    label.setFont(juce::FontOptions(9.5f, juce::Font::plain));
    label.setColour(juce::Label::textColourId, juce::Colour(0xff94a3b8));
    label.setJustificationType(juce::Justification::centredLeft);
    contentContainer->addAndMakeVisible(label);
}

void BuiltInDspComponent::updateAllUI()
{
    if (dspProcessor == nullptr) return;

    // Gate
    const bool gEn = dspProcessor->isGateEnabled();
    gatePwrButton.setButtonText(gEn ? "ON" : "OFF");
    gatePwrButton.setColour(juce::TextButton::buttonColourId, gEn ? juce::Colour(0xff059669) : juce::Colour(0xff334155));
    gateThreshSlider.setValue(dspProcessor->getGateThresholdDb(), juce::dontSendNotification);

    // EQ
    const bool eqEn = dspProcessor->isEqEnabled();
    eqPwrButton.setButtonText(eqEn ? "ON" : "OFF");
    eqPwrButton.setColour(juce::TextButton::buttonColourId, eqEn ? juce::Colour(0xff059669) : juce::Colour(0xff334155));
    eqLowSlider.setValue(dspProcessor->getEqLowGainDb(), juce::dontSendNotification);
    eqMidSlider.setValue(dspProcessor->getEqMidGainDb(), juce::dontSendNotification);
    eqHighSlider.setValue(dspProcessor->getEqHighGainDb(), juce::dontSendNotification);

    // Comp
    const bool cEn = dspProcessor->isCompEnabled();
    compPwrButton.setButtonText(cEn ? "ON" : "OFF");
    compPwrButton.setColour(juce::TextButton::buttonColourId, cEn ? juce::Colour(0xff059669) : juce::Colour(0xff334155));
    compThreshSlider.setValue(dspProcessor->getCompThresholdDb(), juce::dontSendNotification);
    compRatioSlider.setValue(dspProcessor->getCompRatio(), juce::dontSendNotification);
    compMakeupSlider.setValue(dspProcessor->getCompMakeupDb(), juce::dontSendNotification);

    // Reverb
    const bool rEn = dspProcessor->isReverbEnabled();
    reverbPwrButton.setButtonText(rEn ? "ON" : "OFF");
    reverbPwrButton.setColour(juce::TextButton::buttonColourId, rEn ? juce::Colour(0xff059669) : juce::Colour(0xff334155));
    reverbSizeSlider.setValue(dspProcessor->getReverbSize() * 100.0, juce::dontSendNotification);
    reverbDampSlider.setValue(dspProcessor->getReverbDamp() * 100.0, juce::dontSendNotification);
    reverbWetSlider.setValue(dspProcessor->getReverbWetMix() * 100.0, juce::dontSendNotification);

    // Delay
    const bool dEn = dspProcessor->isDelayEnabled();
    delayPwrButton.setButtonText(dEn ? "ON" : "OFF");
    delayPwrButton.setColour(juce::TextButton::buttonColourId, dEn ? juce::Colour(0xff059669) : juce::Colour(0xff334155));
    delayTimeSlider.setValue(dspProcessor->getDelayTimeMs(), juce::dontSendNotification);
    delayFeedbackSlider.setValue(dspProcessor->getDelayFeedback() * 100.0, juce::dontSendNotification);
    delayWetSlider.setValue(dspProcessor->getDelayWetMix() * 100.0, juce::dontSendNotification);

    // Limiter
    const bool lEn = dspProcessor->isLimiterEnabled();
    limiterPwrButton.setButtonText(lEn ? "ON" : "OFF");
    limiterPwrButton.setColour(juce::TextButton::buttonColourId, lEn ? juce::Colour(0xff059669) : juce::Colour(0xff334155));
    limiterThreshSlider.setValue(dspProcessor->getLimiterThresholdDb(), juce::dontSendNotification);

    presetComboBox.setSelectedId(static_cast<int>(dspProcessor->getCurrentPreset()) + 1, juce::dontSendNotification);
}

void BuiltInDspComponent::timerCallback()
{
    if (dspProcessor != nullptr)
    {
        gateIsOpenCached = dspProcessor->isGateOpen();
        compGrCached = dspProcessor->getCompGainReductionDb();
        repaint();
    }
}

void BuiltInDspComponent::changeListenerCallback(juce::ChangeBroadcaster*)
{
    updateAllUI();
}

void BuiltInDspComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff0b0f19));
}

void BuiltInDspComponent::resized()
{
    viewport.setBounds(getLocalBounds());

    const int contentW = std::max(180, getWidth() - 12);
    const int totalContentH = 720;
    contentContainer->setBounds(0, 0, contentW, totalContentH);

    int y = 4;
    headerTitleLabel.setBounds(2, y, contentW - 4, 18);
    y += 22;

    presetComboBox.setBounds(6, y, contentW - 12, 24);
    y += 28;

    factoryResetButton.setBounds(6, y, contentW - 12, 20);
    y += 26;

    auto layoutModule = [&](juce::TextButton& pwr, juce::Label& title, auto&& addControlsFunc, int moduleH)
    {
        pwr.setBounds(6, y + 2, 32, 18);
        title.setBounds(42, y + 2, contentW - 46, 18);
        y += 22;
        addControlsFunc();
        y += 6;
    };

    // 1. Gate
    layoutModule(gatePwrButton, gateTitleLabel, [&] {
        gateThreshLabel.setBounds(8, y, 70, 16);
        gateThreshSlider.setBounds(6, y + 16, contentW - 12, 20);
        y += 38;
    }, 60);

    // 2. EQ
    layoutModule(eqPwrButton, eqTitleLabel, [&] {
        eqLowLabel.setBounds(8, y, 90, 14);
        eqLowSlider.setBounds(6, y + 14, contentW - 12, 18);
        y += 34;

        eqMidLabel.setBounds(8, y, 90, 14);
        eqMidSlider.setBounds(6, y + 14, contentW - 12, 18);
        y += 34;

        eqHighLabel.setBounds(8, y, 90, 14);
        eqHighSlider.setBounds(6, y + 14, contentW - 12, 18);
        y += 34;
    }, 125);

    // 3. Comp
    layoutModule(compPwrButton, compTitleLabel, [&] {
        compThreshLabel.setBounds(8, y, 70, 14);
        compThreshSlider.setBounds(6, y + 14, contentW - 12, 18);
        y += 34;

        compRatioLabel.setBounds(8, y, 70, 14);
        compRatioSlider.setBounds(6, y + 14, contentW - 12, 18);
        y += 34;

        compMakeupLabel.setBounds(8, y, 70, 14);
        compMakeupSlider.setBounds(6, y + 14, contentW - 12, 18);
        y += 34;
    }, 125);

    // 4. Reverb
    layoutModule(reverbPwrButton, reverbTitleLabel, [&] {
        reverbSizeLabel.setBounds(8, y, 70, 14);
        reverbSizeSlider.setBounds(6, y + 14, contentW - 12, 18);
        y += 34;

        reverbDampLabel.setBounds(8, y, 70, 14);
        reverbDampSlider.setBounds(6, y + 14, contentW - 12, 18);
        y += 34;

        reverbWetLabel.setBounds(8, y, 70, 14);
        reverbWetSlider.setBounds(6, y + 14, contentW - 12, 18);
        y += 34;
    }, 125);

    // 5. Delay
    layoutModule(delayPwrButton, delayTitleLabel, [&] {
        delayTimeLabel.setBounds(8, y, 70, 14);
        delayTimeSlider.setBounds(6, y + 14, contentW - 12, 18);
        y += 34;

        delayFeedbackLabel.setBounds(8, y, 70, 14);
        delayFeedbackSlider.setBounds(6, y + 14, contentW - 12, 18);
        y += 34;

        delayWetLabel.setBounds(8, y, 70, 14);
        delayWetSlider.setBounds(6, y + 14, contentW - 12, 18);
        y += 34;
    }, 125);

    // 6. Limiter
    layoutModule(limiterPwrButton, limiterTitleLabel, [&] {
        limiterThreshLabel.setBounds(8, y, 70, 14);
        limiterThreshSlider.setBounds(6, y + 14, contentW - 12, 18);
        y += 34;
    }, 60);

    contentContainer->setSize(contentW, y + 10);
}
