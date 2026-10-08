#include "AiVocalProfilerOverlay.h"

AiVocalProfilerOverlay::AiVocalProfilerOverlay(BuiltInDspAudioProcessor& dsp)
    : dspProcessor(dsp), progressBar(progressVal)
{
    // Title
    titleLabel.setText(juce::String::fromUTF8(u8"🎙️ AI VOCAL PROFILER & AUTO-EQ (1-CLICK SOUNDING)"), juce::dontSendNotification);
    titleLabel.setFont(juce::FontOptions(14.0f, juce::Font::bold));
    titleLabel.setColour(juce::Label::textColourId, juce::Colour(0xffa855f7)); // Purple Neon
    titleLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(titleLabel);

    // Close Button
    closeButton.setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
    closeButton.setColour(juce::TextButton::textColourOffId, juce::Colour(0xff94a3b8));
    closeButton.onClick = [this] {
        dspProcessor.cancelVocalProfiling();
        if (onClose) onClose();
    };
    addAndMakeVisible(closeButton);

    // Instruction Label
    instructionLabel.setText(juce::String::fromUTF8(u8"Bấm nút bên dưới và hát hoặc nói thử một câu ngắn vào Micro trong 5 giây.\nAI sẽ tự động đo đạc âm vực, độ đục phòng và độ sáng của Micro để cân chỉnh EQ lý tưởng."), juce::dontSendNotification);
    instructionLabel.setFont(juce::FontOptions(11.0f, juce::Font::plain));
    instructionLabel.setColour(juce::Label::textColourId, juce::Colour(0xffcbd5e1));
    instructionLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(instructionLabel);

    // Start Record Button
    startRecordButton.setButtonText(juce::String::fromUTF8(u8"🎙️ BẮT ĐẦU THU MẪU (5 GIÂY)"));
    startRecordButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff7c3aed)); // Vibrant Violet
    startRecordButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    startRecordButton.onClick = [this] { handleStartRecord(); };
    addAndMakeVisible(startRecordButton);

    // Progress Bar
    progressBar.setColour(juce::ProgressBar::foregroundColourId, juce::Colour(0xff38bdf8));
    progressBar.setColour(juce::ProgressBar::backgroundColourId, juce::Colour(0xff1e293b));
    addAndMakeVisible(progressBar);
    progressBar.setVisible(false);

    // Results Group
    resultsGroup.setText(juce::String::fromUTF8(u8"KẾT QUẢ CHẨN ĐOÁN GIỌNG HÁT"));
    resultsGroup.setColour(juce::GroupComponent::outlineColourId, juce::Colour(0xff334155));
    resultsGroup.setColour(juce::GroupComponent::textColourId, juce::Colour(0xfff59e0b)); // Gold
    addAndMakeVisible(resultsGroup);

    voiceTypeLabel.setText(juce::String::fromUTF8(u8"Đang chờ thu âm mẫu giọng..."), juce::dontSendNotification);
    voiceTypeLabel.setFont(juce::FontOptions(13.0f, juce::Font::bold));
    voiceTypeLabel.setColour(juce::Label::textColourId, juce::Colour(0xff38bdf8));
    voiceTypeLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(voiceTypeLabel);

    diagnosticsLabel.setText("", juce::dontSendNotification);
    diagnosticsLabel.setFont(juce::FontOptions(11.0f, juce::Font::plain));
    diagnosticsLabel.setColour(juce::Label::textColourId, juce::Colour(0xff94a3b8));
    diagnosticsLabel.setJustificationType(juce::Justification::topLeft);
    addAndMakeVisible(diagnosticsLabel);

    // Style Selector
    styleTitleLabel.setText(juce::String::fromUTF8(u8"PHONG CÁCH PHỐI ÂM (PRESET):"), juce::dontSendNotification);
    styleTitleLabel.setFont(juce::FontOptions(10.5f, juce::Font::bold));
    styleTitleLabel.setColour(juce::Label::textColourId, juce::Colour(0xffcbd5e1));
    addAndMakeVisible(styleTitleLabel);

    styleComboBox.addItem(juce::String::fromUTF8(u8"⭐ Studio Master (Tự nhiên, cân bằng phòng thu)"), 1);
    styleComboBox.addItem(juce::String::fromUTF8(u8"🌸 Bolero & Ballad (Ấm áp, dày giọng, ngọt ngào)"), 2);
    styleComboBox.addItem(juce::String::fromUTF8(u8"🚀 Remix & Pop (Sáng bay bổng, lực, cắt đục)"), 3);
    styleComboBox.addItem(juce::String::fromUTF8(u8"🎙️ Streamer & Podcast (Trong vắt, rõ chữ)"), 4);
    styleComboBox.setSelectedId(1, juce::dontSendNotification);
    styleComboBox.onChange = [this] {
        currentStyle = static_cast<AiVocalProfiler::ProfileStyle>(styleComboBox.getSelectedId() - 1);
        updateEqPreview();
    };
    addAndMakeVisible(styleComboBox);

    // EQ Preview Label
    eqPreviewLabel.setText(juce::String::fromUTF8(u8"Đường cong đề xuất: Low: 0 dB | Mid: 0 dB | High: 0 dB"), juce::dontSendNotification);
    eqPreviewLabel.setFont(juce::FontOptions(11.5f, juce::Font::bold));
    eqPreviewLabel.setColour(juce::Label::textColourId, juce::Colour(0xff34d399)); // Emerald
    eqPreviewLabel.setColour(juce::Label::backgroundColourId, juce::Colour(0xff0f172a));
    eqPreviewLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(eqPreviewLabel);

    // Apply Button
    applyButton.setButtonText(juce::String::fromUTF8(u8"✨ ÁP DỤNG VÀO STUDIO EQ"));
    applyButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff059669)); // Emerald Green
    applyButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    applyButton.onClick = [this] { handleApplyEq(); };
    addAndMakeVisible(applyButton);

    updateResultsUI();
    startTimerHz(20);
}

AiVocalProfilerOverlay::~AiVocalProfilerOverlay()
{
    stopTimer();
    dspProcessor.cancelVocalProfiling();
}

void AiVocalProfilerOverlay::handleStartRecord()
{
    dspProcessor.startVocalProfiling();
    progressBar.setVisible(true);
    startRecordButton.setEnabled(false);
    startRecordButton.setButtonText(juce::String::fromUTF8(u8"⏱️ ĐANG LẮNG NGHE GIỌNG HÁT..."));
    voiceTypeLabel.setText(juce::String::fromUTF8(u8"Đang thu âm mẫu (Hãy hát hoặc nói vào Micro)..."), juce::dontSendNotification);
    voiceTypeLabel.setColour(juce::Label::textColourId, juce::Colour(0xfffbbf24));
}

void AiVocalProfilerOverlay::timerCallback()
{
    if (dspProcessor.isVocalProfiling())
    {
        progressVal = static_cast<double>(dspProcessor.getVocalProfilingProgress());
        int remainingSec = std::max(1, static_cast<int>(std::ceil(5.0 * (1.0 - progressVal))));
        startRecordButton.setButtonText(juce::String::fromUTF8(u8"⏱️ ĐANG LẮNG NGHE... (") + juce::String(remainingSec) + "s)");
        repaint();
    }
    else
    {
        if (progressBar.isVisible())
        {
            progressBar.setVisible(false);
            startRecordButton.setEnabled(true);
            startRecordButton.setButtonText(juce::String::fromUTF8(u8"🔄 THU ÂM LẠI (5 GIÂY)"));
            updateResultsUI();
        }
    }
}

void AiVocalProfilerOverlay::updateResultsUI()
{
    const auto& res = dspProcessor.getVocalProfileResult();
    if (res.isValid)
    {
        voiceTypeLabel.setText(juce::String::fromUTF8(u8"🎯 Âm Vực: ") + res.vocalTypeName, juce::dontSendNotification);
        voiceTypeLabel.setColour(juce::Label::textColourId, juce::Colour(0xff34d399)); // Green
        diagnosticsLabel.setText(res.diagnosticSummary, juce::dontSendNotification);
        updateEqPreview();
        applyButton.setEnabled(true);
    }
    else
    {
        if (res.diagnosticSummary.isNotEmpty())
        {
            voiceTypeLabel.setText(juce::String::fromUTF8(u8"⚠️ Chưa hoàn thành"), juce::dontSendNotification);
            voiceTypeLabel.setColour(juce::Label::textColourId, juce::Colour(0xffef4444));
            diagnosticsLabel.setText(res.diagnosticSummary, juce::dontSendNotification);
        }
        else
        {
            voiceTypeLabel.setText(juce::String::fromUTF8(u8"Đang chờ thu âm mẫu giọng..."), juce::dontSendNotification);
            voiceTypeLabel.setColour(juce::Label::textColourId, juce::Colour(0xff94a3b8));
            diagnosticsLabel.setText(juce::String::fromUTF8(u8"• Chưa có dữ liệu phân tích.\n• Bấm nút trên để bắt đầu phân tích tự động."), juce::dontSendNotification);
        }
        applyButton.setEnabled(false);
    }
}

void AiVocalProfilerOverlay::updateEqPreview()
{
    auto gains = dspProcessor.getAiVocalProfiler().getGainsForStyle(currentStyle);
    juce::String preview = juce::String::fromUTF8(u8"Đường cong đề xuất: ")
        + "Low: " + (gains.lowGainDb >= 0.0f ? "+" : "") + juce::String(gains.lowGainDb, 1) + " dB | "
        + "Mid: " + (gains.midGainDb >= 0.0f ? "+" : "") + juce::String(gains.midGainDb, 1) + " dB | "
        + "High: " + (gains.highGainDb >= 0.0f ? "+" : "") + juce::String(gains.highGainDb, 1) + " dB";
    eqPreviewLabel.setText(preview, juce::dontSendNotification);
}

void AiVocalProfilerOverlay::handleApplyEq()
{
    dspProcessor.applyVocalProfileEq(currentStyle);

    juce::AlertWindow::showMessageBoxAsync(
        juce::AlertWindow::InfoIcon,
        juce::String::fromUTF8(u8"Đã Áp Dụng Auto-EQ Thành Công!"),
        juce::String::fromUTF8(u8"Đã cân chỉnh 3 dải tần Low, Mid, High trong Studio EQ theo đúng chất giọng của bạn!"),
        "OK",
        nullptr,
        juce::ModalCallbackFunction::create([this](int) {
            if (onClose) onClose();
        })
    );
}

void AiVocalProfilerOverlay::paint(juce::Graphics& g)
{
    // Dimmed background
    g.fillAll(juce::Colours::black.withAlpha(0.65f));

    // Modal Card
    auto bounds = getLocalBounds().reduced(20, 15).toFloat();
    g.setGradientFill(juce::ColourGradient(
        juce::Colour(0xff1e1b4b), bounds.getCentreX(), bounds.getY(),
        juce::Colour(0xff0f172a), bounds.getCentreX(), bounds.getBottom(), false
    ));
    g.fillRoundedRectangle(bounds, 12.0f);

    // Card Glow Border
    g.setColour(juce::Colour(0xffa855f7).withAlpha(0.60f));
    g.drawRoundedRectangle(bounds, 12.0f, 1.5f);
}

void AiVocalProfilerOverlay::resized()
{
    auto area = getLocalBounds().reduced(35, 25);

    // Top Row: Title + Close Button
    auto topRow = area.removeFromTop(30);
    closeButton.setBounds(topRow.removeFromRight(28).reduced(2));
    titleLabel.setBounds(topRow);

    area.removeFromTop(8);

    // Instruction Label
    instructionLabel.setBounds(area.removeFromTop(34));
    area.removeFromTop(10);

    // Start Record Button + Progress Bar
    startRecordButton.setBounds(area.removeFromTop(38).reduced(40, 0));
    area.removeFromTop(6);
    progressBar.setBounds(area.removeFromTop(14).reduced(40, 0));

    area.removeFromTop(12);

    // Diagnostics Group
    auto groupArea = area.removeFromTop(130);
    resultsGroup.setBounds(groupArea);
    auto groupContent = groupArea.reduced(12, 18);
    voiceTypeLabel.setBounds(groupContent.removeFromTop(20));
    groupContent.removeFromTop(4);
    diagnosticsLabel.setBounds(groupContent);

    area.removeFromTop(10);

    // Style Selector
    auto styleRow = area.removeFromTop(28);
    styleTitleLabel.setBounds(styleRow.removeFromLeft(200));
    styleComboBox.setBounds(styleRow);

    area.removeFromTop(10);

    // EQ Preview Label
    eqPreviewLabel.setBounds(area.removeFromTop(28));

    area.removeFromTop(12);

    // Apply Button
    applyButton.setBounds(area.removeFromTop(38).reduced(30, 0));
}
