#include "BuiltInDspComponent.h"
#include <iomanip>

BuiltInDspComponent::BuiltInDspComponent(GraphManager& graphMgr)
    : graphManager(graphMgr)
{
    dspProcessor = graphManager.getBuiltInDsp();
    if (dspProcessor != nullptr)
        dspProcessor->addChangeListener(this);

    graphManager.getTempoSyncEngine().addChangeListener(this);

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
    presetComboBox.addSeparator();
    presetComboBox.addItem(juce::String::fromUTF8(u8"☁️ Chu Bin (Dance / Trap)"), 7);
    presetComboBox.addItem(juce::String::fromUTF8(u8"☁️ Lệ Quyên (Bolero Trữ Tình)"), 8);
    presetComboBox.addItem(juce::String::fromUTF8(u8"☁️ Đạt G / Vũ (Indie Acoustic)"), 9);
    presetComboBox.addItem(juce::String::fromUTF8(u8"☁️ Vinahouse Party Live"), 10);
    presetComboBox.addSeparator();
    presetComboBox.addItem(juce::String::fromUTF8(u8"⚡ Tắt DSP (Bypass All)"), 5);
    presetComboBox.addItem(juce::String::fromUTF8(u8"🔄 Khôi Phục Mặc Định (Reset)"), 6);
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

    // Global Tempo Controls Bar
    bpmTitleLabel.setText(juce::String::fromUTF8(u8"TEMPO:"), juce::dontSendNotification);
    bpmTitleLabel.setFont(juce::FontOptions(9.5f, juce::Font::bold));
    bpmTitleLabel.setColour(juce::Label::textColourId, juce::Colour(0xff94a3b8));
    contentContainer->addAndMakeVisible(bpmTitleLabel);

    bpmValueLabel.setText("120 BPM", juce::dontSendNotification);
    bpmValueLabel.setFont(juce::FontOptions(10.5f, juce::Font::bold));
    bpmValueLabel.setColour(juce::Label::textColourId, juce::Colour(0xfff59e0b)); // Gold
    bpmValueLabel.setColour(juce::Label::backgroundColourId, juce::Colour(0xff0f172a));
    bpmValueLabel.setJustificationType(juce::Justification::centred);
    contentContainer->addAndMakeVisible(bpmValueLabel);

    bpmDownBtn.setTooltip(juce::String::fromUTF8(u8"Giảm Tempo (-1 BPM)"));
    bpmDownBtn.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff1e293b));
    bpmDownBtn.setColour(juce::TextButton::textColourOffId, juce::Colour(0xffcbd5e1));
    bpmDownBtn.onClick = [this] {
        auto& engine = graphManager.getTempoSyncEngine();
        engine.setBpm(std::max(40.0, engine.getBpm() - 1.0), "Adjust");
        updateAllUI();
    };
    contentContainer->addAndMakeVisible(bpmDownBtn);

    bpmUpBtn.setTooltip(juce::String::fromUTF8(u8"Tăng Tempo (+1 BPM)"));
    bpmUpBtn.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff1e293b));
    bpmUpBtn.setColour(juce::TextButton::textColourOffId, juce::Colour(0xffcbd5e1));
    bpmUpBtn.onClick = [this] {
        auto& engine = graphManager.getTempoSyncEngine();
        engine.setBpm(std::min(240.0, engine.getBpm() + 1.0), "Adjust");
        updateAllUI();
    };
    contentContainer->addAndMakeVisible(bpmUpBtn);

    tapTempoBtn.setTooltip(juce::String::fromUTF8(u8"Nhấp chuột 2-4 lần theo nhịp bài hát để định lượng Tempo (Tap Tempo)"));
    tapTempoBtn.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff4338ca)); // Indigo
    tapTempoBtn.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    tapTempoBtn.onClick = [this] {
        graphManager.getTempoSyncEngine().tapTempo();
        updateAllUI();
    };
    contentContainer->addAndMakeVisible(tapTempoBtn);

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

    // --- 1. AI Noise & Room De-Reverb Shield ---
    setupModuleHeader(aiPwrButton, aiTitleLabel, juce::String::fromUTF8(u8"1. 🛡️ AI NOISE & DE-REVERB"));
    aiPwrButton.onClick = [this] {
        if (dspProcessor != nullptr) {
            const bool nextState = !dspProcessor->isAiDenoiseEnabled();
            dspProcessor->setAiDenoiseEnabled(nextState);
            dspProcessor->setAiDeReverbEnabled(nextState);
            updateAllUI();
        }
    };

    aiDenoiseToggle.setTooltip(juce::String::fromUTF8(u8"Khử sạch tiếng quạt gió, ve sầu, còi xe, tiếng gõ phím bằng mạng nơ-ron AI ngay cả khi đang hát"));
    aiDenoiseToggle.onClick = [this] {
        if (dspProcessor != nullptr) {
            dspProcessor->setAiDenoiseEnabled(!dspProcessor->isAiDenoiseEnabled());
            updateAllUI();
        }
    };
    contentContainer->addAndMakeVisible(aiDenoiseToggle);

    aiDeReverbToggle.setTooltip(juce::String::fromUTF8(u8"Triệt tiêu tiếng dội âm và tiếng vang phòng chưa dán mút tiêu âm giúp giọng hát khô và nét"));
    aiDeReverbToggle.onClick = [this] {
        if (dspProcessor != nullptr) {
            dspProcessor->setAiDeReverbEnabled(!dspProcessor->isAiDeReverbEnabled());
            updateAllUI();
        }
    };
    contentContainer->addAndMakeVisible(aiDeReverbToggle);

    setupSlider(aiDenoiseSlider, aiDenoiseLabel, "AI Denoise", 0.0, 100.0, 1.0, 75.0, "%");
    aiDenoiseSlider.onValueChange = [this] {
        if (dspProcessor != nullptr) dspProcessor->setAiDenoiseAmount(static_cast<float>(aiDenoiseSlider.getValue() * 0.01));
    };

    setupSlider(aiDeReverbSlider, aiDeReverbLabel, "De-Reverb", 0.0, 100.0, 1.0, 40.0, "%");
    aiDeReverbSlider.onValueChange = [this] {
        if (dspProcessor != nullptr) dspProcessor->setAiDeReverbAmount(static_cast<float>(aiDeReverbSlider.getValue() * 0.01));
    };

    aiStatusLabel.setFont(juce::FontOptions(9.5f, juce::Font::bold));
    aiStatusLabel.setColour(juce::Label::textColourId, juce::Colour(0xff34d399)); // Emerald
    aiStatusLabel.setJustificationType(juce::Justification::centredLeft);
    contentContainer->addAndMakeVisible(aiStatusLabel);

    // --- 2. Noise Gate ---
    setupModuleHeader(gatePwrButton, gateTitleLabel, juce::String::fromUTF8(u8"2. NOISE GATE (CHỐNG ỒN)"));
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

    // --- 3. Studio EQ ---
    setupModuleHeader(eqPwrButton, eqTitleLabel, juce::String::fromUTF8(u8"3. STUDIO EQ 3-BAND"));
    eqPwrButton.onClick = [this] {
        if (dspProcessor != nullptr) {
            dspProcessor->setEqEnabled(!dspProcessor->isEqEnabled());
            updateAllUI();
        }
    };

    aiAutoEqButton.setButtonText(juce::String::fromUTF8(u8"✨ AI AUTO-EQ"));
    aiAutoEqButton.setTooltip(juce::String::fromUTF8(u8"Phân tích chất giọng AI & Tự động cân chỉnh đường cong EQ 1-Click"));
    aiAutoEqButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff7c3aed)); // Purple Violet
    aiAutoEqButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    aiAutoEqButton.onClick = [this] { showAiVocalProfilerOverlay(); };
    contentContainer->addAndMakeVisible(aiAutoEqButton);

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

    // --- 4. Warm Comp ---
    setupModuleHeader(compPwrButton, compTitleLabel, juce::String::fromUTF8(u8"4. WARM COMPRESSOR"));
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

    // --- 4. Lush Reverb Module & Smart Auto-Tail ---
    setupModuleHeader(reverbPwrButton, reverbTitleLabel, juce::String::fromUTF8(u8"4. LUSH REVERB (KHÔNG GIAN)"));
    reverbPwrButton.onClick = [this] {
        if (dspProcessor != nullptr) {
            dspProcessor->setReverbEnabled(!dspProcessor->isReverbEnabled());
            updateAllUI();
        }
    };

    reverbSyncToggle.setTooltip(juce::String::fromUTF8(u8"Tự động tính toán đuôi vang (Decay) khép lại chuẩn xác cuối ô nhịp theo Tempo bài hát, giúp giọng bay bổng mà không bao giờ đè mờ câu hát tiếp theo"));
    reverbSyncToggle.onClick = [this] {
        if (dspProcessor != nullptr) {
            dspProcessor->setReverbBpmSync(!dspProcessor->isReverbBpmSync());
            updateAllUI();
        }
    };
    contentContainer->addAndMakeVisible(reverbSyncToggle);

    reverbBarCombo.addItem(juce::String::fromUTF8(u8"1/2 Bar (Fast Rap / EDM)"), 1);
    reverbBarCombo.addItem(juce::String::fromUTF8(u8"1 Bar (Sạch Studio ⭐)"), 2);
    reverbBarCombo.addItem(juce::String::fromUTF8(u8"2 Bars (Dạt Dào Ballad)"), 3);
    reverbBarCombo.addItem(juce::String::fromUTF8(u8"4 Bars (Không Gian Rộng)"), 4);
    reverbBarCombo.setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xff1e293b));
    reverbBarCombo.setColour(juce::ComboBox::textColourId, juce::Colour(0xfff8fafc));
    reverbBarCombo.onChange = [this] {
        if (dspProcessor != nullptr) {
            int sel = reverbBarCombo.getSelectedId() - 1;
            if (sel >= 0 && sel <= 3) {
                dspProcessor->setReverbBarLength(static_cast<TempoSyncEngine::ReverbBarLength>(sel));
                updateAllUI();
            }
        }
    };
    contentContainer->addAndMakeVisible(reverbBarCombo);

    reverbDecayInfoLabel.setFont(juce::FontOptions(9.5f, juce::Font::bold));
    reverbDecayInfoLabel.setColour(juce::Label::textColourId, juce::Colour(0xff38bdf8));
    reverbDecayInfoLabel.setJustificationType(juce::Justification::centredLeft);
    contentContainer->addAndMakeVisible(reverbDecayInfoLabel);

    setupSlider(reverbSizeSlider, reverbSizeLabel, "Room Size", 0.0, 100.0, 1.0, 65.0, "%");
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

    // --- 5. Stereo Delay Module & Smart BPM Sync ---
    setupModuleHeader(delayPwrButton, delayTitleLabel, juce::String::fromUTF8(u8"5. STEREO DELAY / ECHO"));
    delayPwrButton.onClick = [this] {
        if (dspProcessor != nullptr) {
            dspProcessor->setDelayEnabled(!dspProcessor->isDelayEnabled());
            updateAllUI();
        }
    };

    delaySyncToggle.setTooltip(juce::String::fromUTF8(u8"Khóa thời gian nhại Delay chính xác theo phân đoạn phách Tempo bài hát (1/4, 1/8Dotted, 1/8, 1/8Triplet) giúp tiếng nhại nảy tanh tách đúng nhịp trống"));
    delaySyncToggle.onClick = [this] {
        if (dspProcessor != nullptr) {
            dspProcessor->setDelayBpmSync(!dspProcessor->isDelayBpmSync());
            updateAllUI();
        }
    };
    contentContainer->addAndMakeVisible(delaySyncToggle);

    delaySubdivisionCombo.addItem(juce::String::fromUTF8(u8"1/4 Note (500ms @ 120)"), 1);
    delaySubdivisionCombo.addItem(juce::String::fromUTF8(u8"1/8D Dotted (375ms ⭐)"), 2);
    delaySubdivisionCombo.addItem(juce::String::fromUTF8(u8"1/8 Note (250ms @ 120)"), 3);
    delaySubdivisionCombo.addItem(juce::String::fromUTF8(u8"1/8T Triplet (167ms)"), 4);
    delaySubdivisionCombo.addItem(juce::String::fromUTF8(u8"1/16 Note (125ms)"), 5);
    delaySubdivisionCombo.addItem(juce::String::fromUTF8(u8"1/2 Note (1000ms)"), 6);
    delaySubdivisionCombo.setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xff1e293b));
    delaySubdivisionCombo.setColour(juce::ComboBox::textColourId, juce::Colour(0xfff8fafc));
    delaySubdivisionCombo.onChange = [this] {
        if (dspProcessor != nullptr) {
            int sel = delaySubdivisionCombo.getSelectedId() - 1;
            if (sel >= 0 && sel <= 5) {
                dspProcessor->setDelaySubdivision(static_cast<TempoSyncEngine::DelaySubdivision>(sel));
                updateAllUI();
            }
        }
    };
    contentContainer->addAndMakeVisible(delaySubdivisionCombo);

    delayTimeInfoLabel.setFont(juce::FontOptions(9.5f, juce::Font::bold));
    delayTimeInfoLabel.setColour(juce::Label::textColourId, juce::Colour(0xff38bdf8));
    delayTimeInfoLabel.setJustificationType(juce::Justification::centredLeft);
    contentContainer->addAndMakeVisible(delayTimeInfoLabel);

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
    graphManager.getTempoSyncEngine().removeChangeListener(this);
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

    const double currentBpm = graphManager.getTempoSyncEngine().getBpm();
    bpmValueLabel.setText(juce::String(static_cast<int>(std::round(currentBpm))) + " BPM", juce::dontSendNotification);

    // 1. AI Shield
    const bool aiEn = dspProcessor->isAiDenoiseEnabled();
    aiPwrButton.setButtonText(aiEn ? "ON" : "OFF");
    aiPwrButton.setColour(juce::TextButton::buttonColourId, aiEn ? juce::Colour(0xff059669) : juce::Colour(0xff334155));

    aiDenoiseToggle.setButtonText(aiEn ? juce::String::fromUTF8(u8"⚡ AI DENOISE") : juce::String::fromUTF8(u8"AI: TẮT"));
    aiDenoiseToggle.setColour(juce::TextButton::buttonColourId, aiEn ? juce::Colour(0xff0284c7) : juce::Colour(0xff1e293b));
    aiDenoiseToggle.setColour(juce::TextButton::textColourOffId, aiEn ? juce::Colours::white : juce::Colour(0xff94a3b8));

    const bool deRevEn = dspProcessor->isAiDeReverbEnabled();
    aiDeReverbToggle.setButtonText(deRevEn ? juce::String::fromUTF8(u8"🏠 DE-REVERB") : juce::String::fromUTF8(u8"DE-REV: TẮT"));
    aiDeReverbToggle.setColour(juce::TextButton::buttonColourId, deRevEn ? juce::Colour(0xff4338ca) : juce::Colour(0xff1e293b));
    aiDeReverbToggle.setColour(juce::TextButton::textColourOffId, deRevEn ? juce::Colours::white : juce::Colour(0xff94a3b8));

    aiDenoiseSlider.setValue(dspProcessor->getAiDenoiseAmount() * 100.0, juce::dontSendNotification);
    aiDeReverbSlider.setValue(dspProcessor->getAiDeReverbAmount() * 100.0, juce::dontSendNotification);

    // 2. Gate
    const bool gEn = dspProcessor->isGateEnabled();
    gatePwrButton.setButtonText(gEn ? "ON" : "OFF");
    gatePwrButton.setColour(juce::TextButton::buttonColourId, gEn ? juce::Colour(0xff059669) : juce::Colour(0xff334155));
    gateThreshSlider.setValue(dspProcessor->getGateThresholdDb(), juce::dontSendNotification);

    // 3. EQ
    const bool eqEn = dspProcessor->isEqEnabled();
    eqPwrButton.setButtonText(eqEn ? "ON" : "OFF");
    eqPwrButton.setColour(juce::TextButton::buttonColourId, eqEn ? juce::Colour(0xff059669) : juce::Colour(0xff334155));
    eqLowSlider.setValue(dspProcessor->getEqLowGainDb(), juce::dontSendNotification);
    eqMidSlider.setValue(dspProcessor->getEqMidGainDb(), juce::dontSendNotification);
    eqHighSlider.setValue(dspProcessor->getEqHighGainDb(), juce::dontSendNotification);

    // 4. Comp
    const bool cEn = dspProcessor->isCompEnabled();
    compPwrButton.setButtonText(cEn ? "ON" : "OFF");
    compPwrButton.setColour(juce::TextButton::buttonColourId, cEn ? juce::Colour(0xff059669) : juce::Colour(0xff334155));
    compThreshSlider.setValue(dspProcessor->getCompThresholdDb(), juce::dontSendNotification);
    compRatioSlider.setValue(dspProcessor->getCompRatio(), juce::dontSendNotification);
    compMakeupSlider.setValue(dspProcessor->getCompMakeupDb(), juce::dontSendNotification);

    // 5. Reverb
    const bool rEn = dspProcessor->isReverbEnabled();
    reverbPwrButton.setButtonText(rEn ? "ON" : "OFF");
    reverbPwrButton.setColour(juce::TextButton::buttonColourId, rEn ? juce::Colour(0xff059669) : juce::Colour(0xff334155));
    
    const bool rSync = dspProcessor->isReverbBpmSync();
    reverbSyncToggle.setButtonText(rSync ? juce::String::fromUTF8(u8"⚡ AUTO-TAIL") : juce::String::fromUTF8(u8"TAIL: MANUAL"));
    reverbSyncToggle.setColour(juce::TextButton::buttonColourId, rSync ? juce::Colour(0xff0284c7) : juce::Colour(0xff1e293b));
    reverbSyncToggle.setColour(juce::TextButton::textColourOffId, rSync ? juce::Colours::white : juce::Colour(0xff94a3b8));

    reverbBarCombo.setVisible(rSync);
    reverbDecayInfoLabel.setVisible(rSync);
    reverbSizeSlider.setVisible(!rSync);
    reverbSizeLabel.setVisible(!rSync);
    reverbDampSlider.setVisible(!rSync);
    reverbDampLabel.setVisible(!rSync);

    if (rSync)
    {
        reverbBarCombo.setSelectedId(static_cast<int>(dspProcessor->getReverbBarLength()) + 1, juce::dontSendNotification);
        float decaySec = TempoSyncEngine::calculateReverbDecaySec(currentBpm, dspProcessor->getReverbBarLength());
        reverbDecayInfoLabel.setText(juce::String::fromUTF8(u8"⏱️ Đuôi vang: ") + juce::String(decaySec, 2) + "s (" + TempoSyncEngine::getBarLengthName(dspProcessor->getReverbBarLength()) + ")", juce::dontSendNotification);
    }
    else
    {
        reverbSizeSlider.setValue(dspProcessor->getReverbSize() * 100.0, juce::dontSendNotification);
        reverbDampSlider.setValue(dspProcessor->getReverbDamp() * 100.0, juce::dontSendNotification);
    }
    reverbWetSlider.setValue(dspProcessor->getReverbWetMix() * 100.0, juce::dontSendNotification);

    // 6. Delay
    const bool dEn = dspProcessor->isDelayEnabled();
    delayPwrButton.setButtonText(dEn ? "ON" : "OFF");
    delayPwrButton.setColour(juce::TextButton::buttonColourId, dEn ? juce::Colour(0xff059669) : juce::Colour(0xff334155));

    const bool dSync = dspProcessor->isDelayBpmSync();
    delaySyncToggle.setButtonText(dSync ? juce::String::fromUTF8(u8"⚡ BPM SYNC") : juce::String::fromUTF8(u8"SYNC: MANUAL"));
    delaySyncToggle.setColour(juce::TextButton::buttonColourId, dSync ? juce::Colour(0xff0284c7) : juce::Colour(0xff1e293b));
    delaySyncToggle.setColour(juce::TextButton::textColourOffId, dSync ? juce::Colours::white : juce::Colour(0xff94a3b8));

    delaySubdivisionCombo.setVisible(dSync);
    delayTimeInfoLabel.setVisible(dSync);
    delayTimeSlider.setVisible(!dSync);
    delayTimeLabel.setVisible(!dSync);

    if (dSync)
    {
        delaySubdivisionCombo.setSelectedId(static_cast<int>(dspProcessor->getDelaySubdivision()) + 1, juce::dontSendNotification);
        float delayMs = TempoSyncEngine::calculateDelayTimeMs(currentBpm, dspProcessor->getDelaySubdivision());
        delayTimeInfoLabel.setText(juce::String::fromUTF8(u8"⏱️ Độ trễ: ") + juce::String(static_cast<int>(std::round(delayMs))) + " ms (" + TempoSyncEngine::getSubdivisionName(dspProcessor->getDelaySubdivision()) + ")", juce::dontSendNotification);
    }
    else
    {
        delayTimeSlider.setValue(dspProcessor->getDelayTimeMs(), juce::dontSendNotification);
    }

    delayFeedbackSlider.setValue(dspProcessor->getDelayFeedback() * 100.0, juce::dontSendNotification);
    delayWetSlider.setValue(dspProcessor->getDelayWetMix() * 100.0, juce::dontSendNotification);

    // 7. Limiter
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

        if (dspProcessor->isAiDenoiseEnabled())
        {
            float cutDb = dspProcessor->getAiNoiseReductionDb();
            int prob = static_cast<int>(dspProcessor->getAiVoiceProbability() * 100.0f);
            aiStatusLabel.setText(juce::String::fromUTF8(u8"🛡️ Khử: ") + juce::String(cutDb, 1) + " dB | Giọng: " + juce::String(prob) + "%", juce::dontSendNotification);
        }
        else
        {
            aiStatusLabel.setText(juce::String::fromUTF8(u8"🛡️ AI: Tạm dừng (Bypass)"), juce::dontSendNotification);
        }

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
    const int totalContentH = 920;
    contentContainer->setBounds(0, 0, contentW, totalContentH);

    int y = 4;
    headerTitleLabel.setBounds(2, y, contentW - 4, 18);
    y += 22;

    presetComboBox.setBounds(6, y, contentW - 12, 24);
    y += 28;

    // Global Tempo Control Bar
    bpmTitleLabel.setBounds(6, y + 2, 44, 18);
    bpmValueLabel.setBounds(50, y + 1, 56, 20);
    bpmDownBtn.setBounds(110, y + 1, 18, 20);
    bpmUpBtn.setBounds(130, y + 1, 18, 20);
    tapTempoBtn.setBounds(152, y + 1, contentW - 158, 20);
    y += 26;

    factoryResetButton.setBounds(6, y, contentW - 12, 20);
    y += 26;

    auto layoutModule = [&](juce::TextButton& pwr, juce::Label& title, auto&& addControlsFunc)
    {
        pwr.setBounds(6, y + 2, 32, 18);
        title.setBounds(42, y + 2, contentW - 46, 18);
        y += 22;
        addControlsFunc();
        y += 6;
    };

    // 1. AI Noise & Room De-Reverb Shield
    layoutModule(aiPwrButton, aiTitleLabel, [&] {
        const int btnW = (contentW - 16) / 2;
        aiDenoiseToggle.setBounds(6, y, btnW, 20);
        aiDeReverbToggle.setBounds(8 + btnW, y, btnW, 20);
        y += 24;

        aiDenoiseLabel.setBounds(8, y, 70, 14);
        aiDenoiseSlider.setBounds(6, y + 14, contentW - 12, 18);
        y += 34;

        aiDeReverbLabel.setBounds(8, y, 70, 14);
        aiDeReverbSlider.setBounds(6, y + 14, contentW - 12, 18);
        y += 34;

        aiStatusLabel.setBounds(8, y, contentW - 16, 16);
        y += 18;
    });

    // 2. Gate
    layoutModule(gatePwrButton, gateTitleLabel, [&] {
        gateThreshLabel.setBounds(8, y, 70, 16);
        gateThreshSlider.setBounds(6, y + 16, contentW - 12, 20);
        y += 38;
    });

    // 3. EQ
    layoutModule(eqPwrButton, eqTitleLabel, [&] {
        aiAutoEqButton.setBounds(6, y, contentW - 12, 22);
        y += 26;

        eqLowLabel.setBounds(8, y, 90, 14);
        eqLowSlider.setBounds(6, y + 14, contentW - 12, 18);
        y += 34;

        eqMidLabel.setBounds(8, y, 90, 14);
        eqMidSlider.setBounds(6, y + 14, contentW - 12, 18);
        y += 34;

        eqHighLabel.setBounds(8, y, 90, 14);
        eqHighSlider.setBounds(6, y + 14, contentW - 12, 18);
        y += 34;
    });

    // 4. Comp
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
    });

    // 5. Reverb & Smart Auto-Tail
    layoutModule(reverbPwrButton, reverbTitleLabel, [&] {
        reverbSyncToggle.setBounds(6, y, contentW - 12, 20);
        y += 24;

        const bool rSync = dspProcessor ? dspProcessor->isReverbBpmSync() : true;
        if (rSync)
        {
            reverbBarCombo.setBounds(6, y, contentW - 12, 22);
            y += 24;
            reverbDecayInfoLabel.setBounds(8, y, contentW - 16, 16);
            y += 20;
        }
        else
        {
            reverbSizeLabel.setBounds(8, y, 70, 14);
            reverbSizeSlider.setBounds(6, y + 14, contentW - 12, 18);
            y += 34;

            reverbDampLabel.setBounds(8, y, 70, 14);
            reverbDampSlider.setBounds(6, y + 14, contentW - 12, 18);
            y += 34;
        }

        reverbWetLabel.setBounds(8, y, 70, 14);
        reverbWetSlider.setBounds(6, y + 14, contentW - 12, 18);
        y += 34;
    });

    // 6. Delay & Smart BPM Sync
    layoutModule(delayPwrButton, delayTitleLabel, [&] {
        delaySyncToggle.setBounds(6, y, contentW - 12, 20);
        y += 24;

        const bool dSync = dspProcessor ? dspProcessor->isDelayBpmSync() : true;
        if (dSync)
        {
            delaySubdivisionCombo.setBounds(6, y, contentW - 12, 22);
            y += 24;
            delayTimeInfoLabel.setBounds(8, y, contentW - 16, 16);
            y += 20;
        }
        else
        {
            delayTimeLabel.setBounds(8, y, 70, 14);
            delayTimeSlider.setBounds(6, y + 14, contentW - 12, 18);
            y += 34;
        }

        delayFeedbackLabel.setBounds(8, y, 70, 14);
        delayFeedbackSlider.setBounds(6, y + 14, contentW - 12, 18);
        y += 34;

        delayWetLabel.setBounds(8, y, 70, 14);
        delayWetSlider.setBounds(6, y + 14, contentW - 12, 18);
        y += 34;
    });

    // 7. Limiter
    layoutModule(limiterPwrButton, limiterTitleLabel, [&] {
        limiterThreshLabel.setBounds(8, y, 70, 14);
        limiterThreshSlider.setBounds(6, y + 14, contentW - 12, 18);
        y += 34;
    });

    contentContainer->setSize(contentW, y + 12);

    if (profilerOverlay != nullptr)
    {
        if (auto* top = getTopLevelComponent())
            profilerOverlay->setBounds(top->getLocalBounds());
        else
            profilerOverlay->setBounds(getLocalBounds());
    }
}

void BuiltInDspComponent::showAiVocalProfilerOverlay()
{
    if (dspProcessor == nullptr) return;

    profilerOverlay = std::make_unique<AiVocalProfilerOverlay>(*dspProcessor);
    profilerOverlay->onClose = [this] {
        profilerOverlay.reset();
        updateAllUI();
        repaint();
    };

    if (auto* top = getTopLevelComponent())
    {
        top->addAndMakeVisible(*profilerOverlay);
        profilerOverlay->setBounds(top->getLocalBounds());
    }
    else
    {
        addAndMakeVisible(*profilerOverlay);
        profilerOverlay->setBounds(getLocalBounds());
    }
}
