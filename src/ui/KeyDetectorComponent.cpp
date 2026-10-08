#include "KeyDetectorComponent.h"
#include <iomanip>
#include <sstream>

namespace
{
    juce::String formatTime(double seconds)
    {
        if (seconds < 0.0 || std::isnan(seconds)) seconds = 0.0;
        int totalSec = static_cast<int>(seconds);
        int mins = totalSec / 60;
        int secs = totalSec % 60;
        return juce::String::formatted("%02d:%02d", mins, secs);
    }
}

KeyDetectorComponent::KeyDetectorComponent(GraphManager& graphMgr)
    : graphManager(graphMgr)
{
    beatPlayer = graphManager.getBeatPlayer();
    if (beatPlayer != nullptr)
    {
        beatPlayer->addChangeListener(this);
    }

    if (auto* dsp = graphManager.getBuiltInDsp())
    {
        dsp->addChangeListener(this);
    }

    graphManager.getTempoSyncEngine().addChangeListener(this);

    // --- Load Beat Button ---
    loadBeatButton.setButtonText(juce::String::fromUTF8(u8"NẠP BEAT"));
    loadBeatButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff1e293b));
    loadBeatButton.setColour(juce::TextButton::textColourOffId, juce::Colour(0xff38bdf8));
    loadBeatButton.onClick = [this] { handleLoadBeat(); };
    addAndMakeVisible(loadBeatButton);

    // --- Transport Buttons (Vector Icons) ---
    playPauseButton.setTooltip(juce::String::fromUTF8(u8"Phát / Tạm dừng Beat (Phím cách / Spacebar)"));
    playPauseButton.onClick = [this] { togglePlayPause(); };
    addAndMakeVisible(playPauseButton);

    stopButton.setTooltip(juce::String::fromUTF8(u8"Dừng phát Beat và quay lại đầu bài (Stop)"));
    stopButton.onClick = [this] {
        if (beatPlayer != nullptr)
        {
            beatPlayer->stop();
            updateTransportUI();
        }
    };
    addAndMakeVisible(stopButton);

    loopButton.setTooltip(juce::String::fromUTF8(u8"Bật / Tắt lặp lại bài hát (Loop)"));
    loopButton.onClick = [this] {
        if (beatPlayer != nullptr)
        {
            bool nextLoop = !beatPlayer->isLooping();
            beatPlayer->setLooping(nextLoop);
            loopButton.setLoopActive(nextLoop);
        }
    };
    addAndMakeVisible(loopButton);

    // --- Position Slider ---
    positionSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    positionSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    positionSlider.setRange(0.0, 1.0, 0.001);
    positionSlider.setColour(juce::Slider::trackColourId, juce::Colour(0xff0284c7));
    positionSlider.setColour(juce::Slider::backgroundColourId, juce::Colour(0xff1e293b));
    positionSlider.setColour(juce::Slider::thumbColourId, juce::Colour(0xff38bdf8));
    positionSlider.onDragStart = [this] { isDraggingPosition = true; };
    positionSlider.onDragEnd = [this] {
        if (beatPlayer != nullptr)
        {
            double total = beatPlayer->getTotalLength();
            if (total > 0.0)
            {
                beatPlayer->setPosition(positionSlider.getValue() * total);
            }
        }
        isDraggingPosition = false;
    };
    addAndMakeVisible(positionSlider);

    // --- Time Label ---
    timeLabel.setText("00:00 / 00:00", juce::dontSendNotification);
    timeLabel.setFont(juce::FontOptions(11.0f, juce::Font::bold));
    timeLabel.setColour(juce::Label::textColourId, juce::Colour(0xff94a3b8));
    timeLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(timeLabel);

    // --- Volume Slider ---
    volumeSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    volumeSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    volumeSlider.setRange(0.0, 1.5, 0.01);
    volumeSlider.setValue(1.0);
    volumeSlider.setColour(juce::Slider::trackColourId, juce::Colour(0xff10b981));
    volumeSlider.setColour(juce::Slider::backgroundColourId, juce::Colour(0xff1e293b));
    volumeSlider.setColour(juce::Slider::thumbColourId, juce::Colour(0xff34d399));
    volumeSlider.onValueChange = [this] {
        if (beatPlayer != nullptr)
        {
            beatPlayer->setGainLinear(static_cast<float>(volumeSlider.getValue()));
        }
    };
    addAndMakeVisible(volumeSlider);

    volumeLabel.setText("VOL", juce::dontSendNotification);
    volumeLabel.setFont(juce::FontOptions(10.0f, juce::Font::bold));
    volumeLabel.setColour(juce::Label::textColourId, juce::Colour(0xff64748b));
    volumeLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(volumeLabel);

    // --- Smart Voice Ducking Button ---
    duckingButton.setTooltip(juce::String::fromUTF8(u8"Tự động hạ nhỏ âm lượng beat khi nói vào Micro (Smart Voice Ducking)\n• Click trái: Bật / Tắt Ducking\n• Click phải: Cài đặt mức giảm dB, độ nhạy Micro, thời gian giữ tiếng..."));
    duckingButton.onToggle = [this] {
        if (beatPlayer != nullptr)
        {
            beatPlayer->setDuckingEnabled(!beatPlayer->isDuckingEnabled());
            updateDuckingButtonUI();
        }
    };
    duckingButton.onRightClick = [this] {
        showDuckingSettingsMenu();
    };
    addAndMakeVisible(duckingButton);
    updateDuckingButtonUI();

    // --- Quick 1-Touch AI Denoise & Room De-Reverb Shield Button ---
    aiShieldButton.setTooltip(juce::String::fromUTF8(u8"Khử ồn & triệt tiêu dội âm phòng AI thời gian thực (DeepFilter AI Shield)\n• Click trái: Bật / Tắt AI Shield\n• Click phải: Chọn cường độ khử ồn (Nhẹ 40%, Studio 75%, Mạnh 90%) hoặc bật/tắt De-Reverb..."));
#if HOSI_PRO_EDITION
    aiShieldButton.onToggle = [this] {
        auto* dsp = graphManager.getBuiltInDsp();
        if (dsp != nullptr)
        {
            const bool currentlyOn = dsp->isAiDenoiseEnabled();
            const bool nextState = !currentlyOn;
            dsp->setAiDenoiseEnabled(nextState);
            dsp->setAiDeReverbEnabled(nextState);
            updateAiShieldButtonUI();
        }
    };
    aiShieldButton.onRightClick = [this] {
        showAiShieldSettingsMenu();
    };
#else
    aiShieldButton.onToggle = [this] {
        juce::AlertWindow::showMessageBoxAsync(
            juce::AlertWindow::InfoIcon,
            juce::String::fromUTF8(u8"🔥 TÍNH NĂNG PRO EXCLUSIVE"),
            juce::String::fromUTF8(u8"Lá chắn AI Noise & Room De-Reverb Shield thời gian thực độc quyền trên LiveStream Micro-DAW PRO v3.0!\n\nGiúp khử sạch tiếng ồn môi trường, quạt gió, ve kêu và triệt tiêu dội âm phòng khi hát live. Hãy nâng cấp PRO để trải nghiệm."),
            juce::String::fromUTF8(u8"Đã Hiểu")
        );
    };
    aiShieldButton.onRightClick = [this] {
        juce::AlertWindow::showMessageBoxAsync(
            juce::AlertWindow::InfoIcon,
            juce::String::fromUTF8(u8"🔥 TÍNH NĂNG PRO EXCLUSIVE"),
            juce::String::fromUTF8(u8"Lá chắn AI Noise & Room De-Reverb Shield thời gian thực độc quyền trên LiveStream Micro-DAW PRO v3.0!"),
            juce::String::fromUTF8(u8"Đã Hiểu")
        );
    };
#endif
    addAndMakeVisible(aiShieldButton);
    updateAiShieldButtonUI();

    // --- Quick 1-Touch 24-bit Audio Recorder ---
    recButton.setButtonText(juce::String::fromUTF8(u8"● REC"));
    recButton.setTooltip(juce::String::fromUTF8(u8"Thu âm 1 chạm chuẩn WAV 24-bit (Thu đồng thời Master Mix và Mic mộc Dry)"));
    recButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff1e293b));
    recButton.setColour(juce::TextButton::textColourOffId, juce::Colour(0xffef4444)); // Bright Red
    recButton.onClick = [this] { handleToggleRecord(); };
    addAndMakeVisible(recButton);

    recFolderButton.setButtonText("DIR");
    recFolderButton.setTooltip(juce::String::fromUTF8(u8"Mở thư mục chứa file thu âm (recordings/)"));
    recFolderButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff1e293b));
    recFolderButton.setColour(juce::TextButton::textColourOffId, juce::Colour(0xff94a3b8));
    recFolderButton.onClick = [this] {
        auto recDir = graphManager.getAudioRecorder().getRecordingsFolder();
        recDir.startAsProcess();
    };
    addAndMakeVisible(recFolderButton);

    // --- Key Display & Labels ---
    keyTitleLabel.setText(juce::String::fromUTF8(u8"DÒ TONE BEAT"), juce::dontSendNotification);
    keyTitleLabel.setFont(juce::FontOptions(10.0f, juce::Font::bold));
    keyTitleLabel.setColour(juce::Label::textColourId, juce::Colour(0xff64748b));
    keyTitleLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(keyTitleLabel);

    keyDisplayLabel.setText("--", juce::dontSendNotification);
    keyDisplayLabel.setFont(juce::FontOptions(18.0f, juce::Font::bold));
    keyDisplayLabel.setColour(juce::Label::textColourId, juce::Colour(0xfff59e0b)); // Glowing amber/gold
    keyDisplayLabel.setJustificationType(juce::Justification::centredLeft);
    keyDisplayLabel.setTooltip(juce::String::fromUTF8(u8"Tone bài hát đang được nhận diện. Bấm 'CHỌN TONE' để đổi thủ công."));
    addAndMakeVisible(keyDisplayLabel);

    confidenceLabel.setText(juce::String::fromUTF8(u8"Chờ tín hiệu nhạc..."), juce::dontSendNotification);
    confidenceLabel.setFont(juce::FontOptions(10.0f, juce::Font::plain));
    confidenceLabel.setColour(juce::Label::textColourId, juce::Colour(0xff94a3b8));
    confidenceLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(confidenceLabel);

    // --- Auto-Push Toggle Button ---
    autoPushToggle.setTooltip(juce::String::fromUTF8(u8"Tự động nạp Tone vào Auto-Tune ngay khi chốt tone (Confidence >= 75%) mà không cần bấm thủ công\n• ⚡ AUTO (Xanh lá): Tự động nạp ngay khi chốt Tone\n• AUTO: TẮT (Tối): Cần bấm nút ĐỒNG BỘ thủ công"));
    autoPushToggle.onClick = [this] {
        isAutoPushEnabled = !isAutoPushEnabled;
        updateAutoPushUI();
    };
    addAndMakeVisible(autoPushToggle);
    updateAutoPushUI();

    // --- Sync to Auto-Tune Button ---
    syncToAutoTuneButton.setButtonText(juce::String::fromUTF8(u8"ĐỒNG BỘ AUTO-TUNE"));
    syncToAutoTuneButton.setTooltip(juce::String::fromUTF8(u8"Tự động nạp Tone đang hiển thị vào Plugin Auto-Tune trong Rack"));
    syncToAutoTuneButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff4f46e5)); // Indigo
    syncToAutoTuneButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    syncToAutoTuneButton.onClick = [this] { syncKeyToPitchPlugin(); };
    addAndMakeVisible(syncToAutoTuneButton);

    // --- Fallback Manual Key Selector Button ---
    manualKeyButton.setButtonText(juce::String::fromUTF8(u8"CHỌN TONE"));
    manualKeyButton.setTooltip(juce::String::fromUTF8(u8"Chọn nhanh Tone thủ công từ danh sách 24 giọng (Major/Minor) để gửi ngay vào Auto-Tune"));
    manualKeyButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff334155));
    manualKeyButton.setColour(juce::TextButton::textColourOffId, juce::Colour(0xfffcd34d));
    manualKeyButton.onClick = [this] { showManualKeySelectMenu(); };
    addAndMakeVisible(manualKeyButton);

    // --- Tempo / BPM Button & Tap Tempo ---
    bpmButton.setTooltip(juce::String::fromUTF8(u8"Tempo bài hát (BPM). Click để đổi tốc độ chuẩn theo thể loại hoặc nhập số"));
    bpmButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff1e293b));
    bpmButton.setColour(juce::TextButton::textColourOffId, juce::Colour(0xfff59e0b)); // Gold
#if HOSI_PRO_EDITION
    bpmButton.onClick = [this] { showBpmSettingsMenu(); };
#else
    bpmButton.onClick = [this] {
        juce::AlertWindow::showMessageBoxAsync(
            juce::AlertWindow::InfoIcon,
            juce::String::fromUTF8(u8"🔥 TÍNH NĂNG PRO EXCLUSIVE"),
            juce::String::fromUTF8(u8"Bộ đồng bộ nhịp tự động (Smart Tempo / BPM Sync Engine) đồng bộ hiệu ứng Reverb & Delay theo phách nhịp độc quyền trên LiveStream Micro-DAW PRO v3.0!\n\nHãy nâng cấp phiên bản PRO để sử dụng."),
            juce::String::fromUTF8(u8"Đã Hiểu")
        );
    };
#endif
    addAndMakeVisible(bpmButton);

    tapTempoButton.setTooltip(juce::String::fromUTF8(u8"Nhấp chuột 2-4 lần theo nhịp bài hát để định lượng Tempo (Tap Tempo)"));
    tapTempoButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff312e81)); // Dark indigo
    tapTempoButton.setColour(juce::TextButton::textColourOffId, juce::Colour(0xffa5b4fc));
#if HOSI_PRO_EDITION
    tapTempoButton.onClick = [this] {
        graphManager.getTempoSyncEngine().tapTempo();
        updateKeyUI();
    };
#else
    tapTempoButton.onClick = [this] {
        juce::AlertWindow::showMessageBoxAsync(
            juce::AlertWindow::InfoIcon,
            juce::String::fromUTF8(u8"🔥 TÍNH NĂNG PRO EXCLUSIVE"),
            juce::String::fromUTF8(u8"Tính năng Tap Tempo định lượng nhịp tự động độc quyền trên LiveStream Micro-DAW PRO v3.0!"),
            juce::String::fromUTF8(u8"Đã Hiểu")
        );
    };
#endif
    addAndMakeVisible(tapTempoButton);

    // --- Source Toggle ---
    sourceToggleButton.setButtonText(juce::String::fromUTF8(u8"NGUỒN: BEAT"));
    sourceToggleButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff1e293b));
    sourceToggleButton.setColour(juce::TextButton::textColourOffId, juce::Colour(0xffcbd5e1));
    sourceToggleButton.onClick = [this] {
        if (beatPlayer != nullptr)
        {
            auto currentSrc = beatPlayer->getAnalysisSource();
            if (currentSrc == BeatPlayerAudioProcessor::AnalysisSource::BeatPlayer)
            {
                beatPlayer->setAnalysisSource(BeatPlayerAudioProcessor::AnalysisSource::LiveMicMaster);
                sourceToggleButton.setButtonText(juce::String::fromUTF8(u8"NGUỒN: MIC LIVE"));
                sourceToggleButton.setColour(juce::TextButton::textColourOffId, juce::Colour(0xfffbbf24));
            }
            else
            {
                beatPlayer->setAnalysisSource(BeatPlayerAudioProcessor::AnalysisSource::BeatPlayer);
                sourceToggleButton.setButtonText(juce::String::fromUTF8(u8"NGUỒN: BEAT"));
                sourceToggleButton.setColour(juce::TextButton::textColourOffId, juce::Colour(0xff38bdf8));
            }
        }
    };
    addAndMakeVisible(sourceToggleButton);

    startTimerHz(25); // 25 FPS UI refresh
}

KeyDetectorComponent::~KeyDetectorComponent()
{
    stopTimer();
    graphManager.getTempoSyncEngine().removeChangeListener(this);
    if (auto* dsp = graphManager.getBuiltInDsp())
    {
        dsp->removeChangeListener(this);
    }
    if (beatPlayer != nullptr)
    {
        beatPlayer->removeChangeListener(this);
    }
}

void KeyDetectorComponent::handleLoadBeat()
{
    fileChooser = std::make_unique<juce::FileChooser>(
        juce::String::fromUTF8(u8"Chọn File Nhạc Beat / Karaoke (MP3, WAV, FLAC, AIFF)..."),
        juce::File::getSpecialLocation(juce::File::userMusicDirectory),
        "*.mp3;*.wav;*.flac;*.ogg;*.aiff"
    );

    auto flags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles;
    fileChooser->launchAsync(flags, [this](const juce::FileChooser& fc) {
        auto result = fc.getResult();
        if (result.existsAsFile() && beatPlayer != nullptr)
        {
            juce::String err;
            if (beatPlayer->loadAudioFile(result, err))
            {
                lastAutoPushedKey = "";
                loadBeatButton.setButtonText(result.getFileName().substring(0, 14) + (result.getFileName().length() > 14 ? ".." : ""));
                updateTransportUI();
                updateKeyUI();
            }
            else
            {
                juce::AlertWindow::showMessageBoxAsync(
                    juce::AlertWindow::WarningIcon,
                    juce::String::fromUTF8(u8"Lỗi nạp file Beat"),
                    err,
                    "OK"
                );
            }
        }
    });
}

void KeyDetectorComponent::togglePlayPause()
{
    if (beatPlayer == nullptr) return;

    if (beatPlayer->isPlaying())
    {
        beatPlayer->pause();
    }
    else
    {
        beatPlayer->play();
    }
    updateTransportUI();
}

void KeyDetectorComponent::updateTransportUI()
{
    if (beatPlayer == nullptr) return;

    const bool playing = beatPlayer->isPlaying();
    playPauseButton.setPlaying(playing);
    loopButton.setLoopActive(beatPlayer->isLooping());

    double current = beatPlayer->getCurrentPosition();
    double total = beatPlayer->getTotalLength();

    timeLabel.setText(formatTime(current) + " / " + formatTime(total), juce::dontSendNotification);

    if (!isDraggingPosition && total > 0.0)
    {
        positionSlider.setValue(current / total, juce::dontSendNotification);
    }
}

void KeyDetectorComponent::updateKeyUI()
{
    const double currentBpm = graphManager.getTempoSyncEngine().getBpm();
    bpmButton.setButtonText(juce::String(static_cast<int>(std::round(currentBpm))) + " BPM");

    // Real-Time Beat Estimation Auto-Sync check
    auto beatRes = graphManager.getTempoSyncEngine().getDetectedBeatResult();

    if (beatPlayer == nullptr) return;

    auto result = beatPlayer->getDetectedKey();
    if (result.scale != KeyDetector::ScaleType::Unknown && result.confidence > 0.15f)
    {
        keyDisplayLabel.setText(result.keyName, juce::dontSendNotification);
        
        int percent = static_cast<int>(result.confidence * 100.0f);
        if (result.isLocked)
        {
            confidenceLabel.setText(juce::String::fromUTF8(u8"Đã chốt: ") + juce::String(percent) + "%", juce::dontSendNotification);
            confidenceLabel.setColour(juce::Label::textColourId, juce::Colour(0xff34d399)); // Emerald Green

            // Auto-push to Auto-Tune when locked and tone is new
            if (isAutoPushEnabled && result.keyName != lastAutoPushedKey)
            {
                lastAutoPushedKey = result.keyName;
                applyKeyToAutoTune(result.rootNote, result.scale, juce::String::fromUTF8(u8"Beat Dò Tone (Tự động)"), false);
            }
        }
        else
        {
            confidenceLabel.setText(juce::String::fromUTF8(u8"Đang dò: ") + juce::String(percent) + "%", juce::dontSendNotification);
            confidenceLabel.setColour(juce::Label::textColourId, (percent >= 70) ? juce::Colour(0xff34d399) : juce::Colour(0xfffbbf24));
        }
    }
    else
    {
        keyDisplayLabel.setText(result.keyName, juce::dontSendNotification);
        confidenceLabel.setText(juce::String::fromUTF8(u8"Chờ tín hiệu nhạc..."), juce::dontSendNotification);
        confidenceLabel.setColour(juce::Label::textColourId, juce::Colour(0xff94a3b8));
    }

    // Smooth Chroma Bars
    for (int i = 0; i < 12; ++i)
    {
        smoothedChroma[i] = smoothedChroma[i] * 0.75f + result.chromaProfile[i] * 0.25f;
    }
}

void KeyDetectorComponent::showBpmSettingsMenu()
{
#if !HOSI_PRO_EDITION
    juce::AlertWindow::showMessageBoxAsync(
        juce::AlertWindow::InfoIcon,
        juce::String::fromUTF8(u8"🔥 TÍNH NĂNG PRO EXCLUSIVE"),
        juce::String::fromUTF8(u8"Bộ đồng bộ nhịp tự động (Smart Tempo / BPM Sync Engine) đồng bộ hiệu ứng Reverb & Delay theo phách nhịp độc quyền trên LiveStream Micro-DAW PRO v3.0!\n\nHãy nâng cấp phiên bản PRO để sử dụng."),
        juce::String::fromUTF8(u8"Đã Hiểu")
    );
    return;
#else
    juce::PopupMenu menu;
    menu.addSectionHeader(juce::String::fromUTF8(u8"CÀI ĐẶT TEMPO / BPM (SMART SYNC DELAY & REVERB)"));

    menu.addItem(1, juce::String::fromUTF8(u8"⚡ 65 BPM - Bolero Chậm Rãi"));
    menu.addItem(2, juce::String::fromUTF8(u8"⚡ 75 BPM - Ballad Trữ Tình (Tiêu Chuẩn)"));
    menu.addItem(3, juce::String::fromUTF8(u8"⚡ 90 BPM - R&B / Acoustic Pop"));
    menu.addItem(4, juce::String::fromUTF8(u8"⚡ 105 BPM - Pop Dance / Disco"));
    menu.addItem(5, juce::String::fromUTF8(u8"⚡ 120 BPM - Nhạc Trẻ Sôi Động ⭐"));
    menu.addItem(6, juce::String::fromUTF8(u8"⚡ 128 BPM - Vinahouse / Remix Club 🔥"));
    menu.addItem(7, juce::String::fromUTF8(u8"⚡ 132 BPM - EDM / Electro Festival"));
    menu.addItem(8, juce::String::fromUTF8(u8"⚡ 140 BPM - Trap / Hip-Hop Fast"));

    menu.addSeparator();
    menu.addItem(100, juce::String::fromUTF8(u8"✏️ Nhập Số BPM Tùy Chỉnh..."));

    menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(&bpmButton), [this](int result) {
        if (result == 0) return;

        auto& engine = graphManager.getTempoSyncEngine();
        switch (result)
        {
        case 1: engine.setBpm(65.0, "Bolero Preset"); break;
        case 2: engine.setBpm(75.0, "Ballad Preset"); break;
        case 3: engine.setBpm(90.0, "R&B Preset"); break;
        case 4: engine.setBpm(105.0, "Pop Dance Preset"); break;
        case 5: engine.setBpm(120.0, "Standard 120"); break;
        case 6: engine.setBpm(128.0, "Remix Preset"); break;
        case 7: engine.setBpm(132.0, "EDM Preset"); break;
        case 8: engine.setBpm(140.0, "Trap Preset"); break;
        case 100:
        {
            auto* dialog = new juce::AlertWindow(
                juce::String::fromUTF8(u8"Nhập Tempo (BPM)"),
                juce::String::fromUTF8(u8"Nhập tốc độ bài hát từ 40 đến 240 BPM:"),
                juce::AlertWindow::QuestionIcon
            );
            dialog->addTextEditor("bpm", juce::String(static_cast<int>(std::round(engine.getBpm()))));
            dialog->addButton("OK", 1, juce::KeyPress(juce::KeyPress::returnKey));
            dialog->addButton(juce::String::fromUTF8(u8"Hủy"), 0, juce::KeyPress(juce::KeyPress::escapeKey));

            dialog->enterModalState(true, juce::ModalCallbackFunction::create([this, dialog](int modalResult) {
                if (modalResult == 1)
                {
                    double val = dialog->getTextEditorContents("bpm").getDoubleValue();
                    if (val >= 40.0 && val <= 260.0)
                    {
                        graphManager.getTempoSyncEngine().setBpm(val, "User Custom Input");
                        updateKeyUI();
                    }
                }
                delete dialog;
            }));
            break;
        }
        default: break;
        }
        updateKeyUI();
    });
#endif
}

void KeyDetectorComponent::showManualKeySelectMenu()
{
    juce::PopupMenu menu;
    menu.addSectionHeader(juce::String::fromUTF8(u8"CHỌN TONE THỦ CÔNG (TRUYỀN VÀO AUTO-TUNE)"));

    juce::PopupMenu majorMenu;
    juce::PopupMenu minorMenu;

    for (int i = 0; i < 12; ++i)
    {
        const juce::String note = KeyDetector::getNoteName(i);
        majorMenu.addItem(100 + i, note + " Major (Trưởng)");
        minorMenu.addItem(200 + i, note + " Minor (Thứ)");
    }

    menu.addSubMenu(juce::String::fromUTF8(u8"♫ Giọng Trưởng (Major Scale)"), majorMenu);
    menu.addSubMenu(juce::String::fromUTF8(u8"♫ Giọng Thứ (Minor Scale)"), minorMenu);

    menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(&manualKeyButton), [this](int result) {
        if (result >= 100 && result < 112)
        {
            int root = result - 100;
            applyKeyToAutoTune(root, KeyDetector::ScaleType::Major, juce::String::fromUTF8(u8"Thủ công"));
        }
        else if (result >= 200 && result < 212)
        {
            int root = result - 200;
            applyKeyToAutoTune(root, KeyDetector::ScaleType::Minor, juce::String::fromUTF8(u8"Thủ công"));
        }
    });
}

void KeyDetectorComponent::applyKeyToAutoTune(int rootNote, KeyDetector::ScaleType scale, const juce::String& sourceName, bool showNotificationPopup)
{
    const juce::String keyFormatted = KeyDetector::formatKeyName(rootNote, scale);
    keyDisplayLabel.setText(keyFormatted, juce::dontSendNotification);
    confidenceLabel.setText(juce::String::fromUTF8(u8"Đã gán: ") + keyFormatted, juce::dontSendNotification);
    confidenceLabel.setColour(juce::Label::textColourId, juce::Colour(0xff34d399));

    bool matched = false;
    juce::String targetPluginName;
    const auto& slots = graphManager.getSlots();

    for (const auto& slot : slots)
    {
        if (slot.node != nullptr && slot.node->getProcessor() != nullptr)
        {
            auto* proc = slot.node->getProcessor();
            const juce::String procName = proc->getName().toLowerCase();

            // Skip Auto-Key itself when applying parameters to Auto-Tune!
            if (procName.contains("auto-key") || procName.contains("autokey") || procName.contains("songkey"))
                continue;

            // Match Auto-Tune / Pitch Correction plugin
            if (procName.contains("tune") || procName.contains("pitch") || procName.contains("scale") || procName.contains("antares"))
            {
                targetPluginName = proc->getName();
                const auto& params = proc->getParameters();
                for (auto* param : params)
                {
                    const juce::String paramName = param->getName(64).toLowerCase().trim();

                    // Filter out unrelated parameters and force Auto Mode (avoid Graph Mode)
                    if (paramName.contains("pitch mode") || 
                        paramName.contains("correction mode") ||
                        paramName.contains("screen mode") ||
                        paramName.contains("view mode") ||
                        paramName.contains("target mode"))
                    {
                        // 0.0f = Auto Mode in Auto-Tune Pro (1.0f = Graph Mode)
                        param->setValueNotifyingHost(0.0f);
                        continue;
                    }

                    if (paramName.contains("input type") || 
                        paramName.contains("source type") ||
                        paramName.contains("classic") ||
                        paramName.contains("formant") ||
                        paramName.contains("tracking") ||
                        paramName.contains("auto-key") ||
                        paramName.contains("autokey") ||
                        paramName.contains("bypass") ||
                        paramName.contains("vibrato") ||
                        paramName.contains("retune") ||
                        paramName.contains("detune") ||
                        paramName.contains("humanize"))
                    {
                        continue;
                    }

                    // 1. Root / Key note parameter
                    const bool isKeyParam = (paramName == "key" || 
                                             paramName == "root" || 
                                             paramName == "root note" || 
                                             paramName == "tonic" || 
                                             paramName == "base note" || 
                                             paramName == "tuning key" ||
                                             (paramName.contains("root") && !paramName.contains("octave") && !paramName.contains("shift")));

                    if (isKeyParam)
                    {
                        const int numSteps = param->getNumSteps();
                        if (numSteps == 12)
                            param->setValueNotifyingHost((static_cast<float>(rootNote) + 0.5f) / 12.0f);
                        else
                            param->setValueNotifyingHost(static_cast<float>(rootNote) / 11.0f);
                        matched = true;
                    }
                    // 2. Scale parameter (strictly matched, not loose 'mode' or 'type')
                    else if (paramName == "scale" || 
                             paramName == "scale type" || 
                             paramName == "target scale" || 
                             paramName == "scale name" || 
                             paramName == "scale mode" ||
                             (paramName.contains("scale") && !paramName.contains("time") && !paramName.contains("ui") && !paramName.contains("display") && !paramName.contains("window")))
                    {
                        const int numSteps = param->getNumSteps();
                        float scaleVal = 0.0f;
                        if (scale == KeyDetector::ScaleType::Minor)
                        {
                            if (numSteps > 1)
                                scaleVal = 1.5f / static_cast<float>(numSteps);
                            else
                                scaleVal = 0.5f;
                        }
                        else
                        {
                            scaleVal = 0.0f; // Major is always 0.0f
                        }
                        param->setValueNotifyingHost(scaleVal);
                        matched = true;
                    }
                }
            }
        }
    }

    // Always copy to clipboard for user convenience
    juce::SystemClipboard::copyTextToClipboard(keyFormatted);

    if (showNotificationPopup)
    {
        if (matched)
        {
            juce::AlertWindow::showMessageBoxAsync(
                juce::AlertWindow::InfoIcon,
                juce::String::fromUTF8(u8"Đã Đồng Bộ Tone Vào Auto-Tune!"),
                juce::String::fromUTF8(u8"Nguồn: [") + sourceName + juce::String::fromUTF8(u8"]\nĐã truyền Tone [") + keyFormatted + juce::String::fromUTF8(u8"] vào plugin [") + targetPluginName + juce::String::fromUTF8(u8"] trong Rack thành công!"),
                "OK"
            );
        }
        else
        {
            juce::AlertWindow::showMessageBoxAsync(
                juce::AlertWindow::InfoIcon,
                juce::String::fromUTF8(u8"Đã Nhận Diện Tone: ") + keyFormatted,
                juce::String::fromUTF8(u8"Nguồn: [") + sourceName + juce::String::fromUTF8(u8"]\nĐã sao chép [") + keyFormatted + juce::String::fromUTF8(u8"] vào bộ nhớ tạm (Clipboard).\nChưa thấy plugin Auto-Tune trong Rack, bạn có thể nạp Auto-Tune vào Slot 1 để đồng bộ tự động!"),
                "OK"
            );
        }
    }
}

void KeyDetectorComponent::updateAutoPushUI()
{
    if (isAutoPushEnabled)
    {
        autoPushToggle.setButtonText(juce::String::fromUTF8(u8"⚡ AUTO"));
        autoPushToggle.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff059669)); // Emerald 600
        autoPushToggle.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    }
    else
    {
        autoPushToggle.setButtonText(juce::String::fromUTF8(u8"AUTO: TẮT"));
        autoPushToggle.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff1e293b));
        autoPushToggle.setColour(juce::TextButton::textColourOffId, juce::Colour(0xff64748b));
    }
}

void KeyDetectorComponent::syncKeyToPitchPlugin()
{
    if (beatPlayer == nullptr) return;

    auto result = beatPlayer->getDetectedKey();
    if (result.scale != KeyDetector::ScaleType::Unknown && result.confidence > 0.15f)
    {
        lastAutoPushedKey = result.keyName;
        applyKeyToAutoTune(result.rootNote, result.scale, juce::String::fromUTF8(u8"Bộ Dò Tích Hợp Micro-DAW"), true);
    }
    else
    {
        juce::AlertWindow::showMessageBoxAsync(
            juce::AlertWindow::InfoIcon,
            juce::String::fromUTF8(u8"Chưa nhận diện được Tone"),
            juce::String::fromUTF8(u8"Vui lòng phát nhạc Beat/Micro để phân tích Tone, hoặc bấm 'CHỌN TONE' để nạp Tone vào Auto-Tune!"),
            "OK"
        );
    }
}

void KeyDetectorComponent::handleToggleRecord()
{
    auto& recorder = graphManager.getAudioRecorder();
    if (recorder.isRecording())
    {
        recorder.stopRecording();
        updateRecordButtonUI();

        auto lastFile = recorder.getLastMasterRecordingFile();
        if (lastFile.existsAsFile())
        {
            juce::String msg = juce::String::fromUTF8(u8"Đã lưu bản thu âm chất lượng cao:\n- ") + lastFile.getFileName();
            juce::AlertWindow::showMessageBoxAsync(
                juce::AlertWindow::InfoIcon,
                juce::String::fromUTF8(u8"Thu Âm Thành Công!"),
                msg,
                "OK"
            );
        }
    }
    else
    {
        juce::String err;
        if (recorder.startRecording(err))
        {
            updateRecordButtonUI();
        }
        else
        {
            juce::AlertWindow::showMessageBoxAsync(
                juce::AlertWindow::WarningIcon,
                juce::String::fromUTF8(u8"Lỗi Bắt Đầu Thu Âm"),
                err,
                "OK"
            );
        }
    }
}

void KeyDetectorComponent::updateRecordButtonUI()
{
    auto& recorder = graphManager.getAudioRecorder();
    const bool isRec = recorder.isRecording();
    recButton.setButtonText(isRec ? juce::String::fromUTF8(u8"■ DỪNG") : juce::String::fromUTF8(u8"● REC"));
    recButton.setColour(juce::TextButton::buttonColourId, isRec ? juce::Colour(0xffdc2626) : juce::Colour(0xff1e293b));
    recButton.setColour(juce::TextButton::textColourOffId, isRec ? juce::Colours::white : juce::Colour(0xffef4444));
}

void KeyDetectorComponent::scanAutoKeyPluginsInRack()
{
    const auto& slots = graphManager.getSlots();
    for (const auto& slot : slots)
    {
        if (slot.node != nullptr && slot.node->getProcessor() != nullptr)
        {
            auto* proc = slot.node->getProcessor();
            const juce::String procName = proc->getName().toLowerCase();

            if (procName.contains("auto-key") || procName.contains("autokey") || procName.contains("songkey") || procName.contains("key detector"))
            {
                int detectedRoot = -1;
                KeyDetector::ScaleType detectedScale = KeyDetector::ScaleType::Unknown;
                double detectedBpm = 0.0;
                bool sendTriggered = false;

                for (auto* param : proc->getParameters())
                {
                    const juce::String pName = param->getName(64).toLowerCase().trim();
                    const float val = param->getValue();

                    // Key / Root note
                    if (pName == "key" || pName == "root" || pName == "detected key" || pName == "tonic" || pName == "root note")
                    {
                        int steps = param->getNumSteps();
                        if (steps <= 1) steps = 12;
                        int rootIdx = static_cast<int>(std::round(val * (steps - 1)));
                        if (rootIdx >= 0 && rootIdx < 12)
                            detectedRoot = rootIdx;
                    }
                    // Scale
                    else if (pName == "scale" || pName == "detected scale" || pName == "scale type" || pName == "scale mode")
                    {
                        if (val > 0.3f)
                            detectedScale = KeyDetector::ScaleType::Minor;
                        else
                            detectedScale = KeyDetector::ScaleType::Major;
                    }
                    // Tempo / BPM
                    else if (pName.contains("tempo") || pName.contains("bpm"))
                    {
                        juce::String textVal = param->getText(val, 16).trim();
                        double bpm = textVal.getDoubleValue();
                        if (bpm < 40.0 || bpm > 260.0)
                        {
                            bpm = 40.0 + (val * 200.0);
                        }
                        if (bpm >= 40.0 && bpm <= 260.0)
                            detectedBpm = bpm;
                    }
                    // Send to Auto-Tune button
                    else if (pName.contains("send") || pName.contains("sync"))
                    {
                        if (val > 0.5f)
                            sendTriggered = true;
                    }
                }

                // If Tone detected
                if (detectedRoot >= 0 && detectedScale != KeyDetector::ScaleType::Unknown)
                {
                    const juce::String keyFormatted = KeyDetector::formatKeyName(detectedRoot, detectedScale);
                    if (keyFormatted != lastAutoKeyPluginKey || sendTriggered)
                    {
                        lastAutoKeyPluginKey = keyFormatted;
                        applyKeyToAutoTune(detectedRoot, detectedScale, proc->getName() + " (VST3)", false);
                    }
                }

                // If Tempo (BPM) detected
                if (detectedBpm >= 40.0 && detectedBpm <= 260.0)
                {
                    if (std::abs(detectedBpm - lastAutoKeyPluginBpm) > 0.5)
                    {
                        lastAutoKeyPluginBpm = detectedBpm;
                        graphManager.getTempoSyncEngine().setBpm(detectedBpm, proc->getName() + " (VST3)");
                    }
                }
            }
        }
    }
}

void KeyDetectorComponent::timerCallback()
{
    updateTransportUI();
    updateKeyUI();
    scanAutoKeyPluginsInRack();
    updateAiShieldButtonUI();
    updateRecordButtonUI();
    repaint();
}

void KeyDetectorComponent::changeListenerCallback(juce::ChangeBroadcaster* /*source*/)
{
    updateTransportUI();
    updateKeyUI();
    updateDuckingButtonUI();
    updateAiShieldButtonUI();
}

void KeyDetectorComponent::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    // Subtle dark gradient background
    g.setGradientFill(juce::ColourGradient(
        juce::Colour(0xff0f172a), 0, 0,
        juce::Colour(0xff090d16), 0, bounds.getHeight(), false
    ));
    g.fillRoundedRectangle(bounds, 8.0f);

    // Border
    g.setColour(juce::Colour(0xff1e293b));
    g.drawRoundedRectangle(bounds.reduced(0.5f), 8.0f, 1.0f);

    // Draw Chroma Visualizer on the right side
    auto chromaArea = getLocalBounds().removeFromRight(150).reduced(6, 8).toFloat();
    const float barWidth = chromaArea.getWidth() / 12.0f;
    const float maxHeight = chromaArea.getHeight() - 14.0f;

    const std::array<juce::String, 12> pitchLabels = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
    
    int activeRoot = (beatPlayer != nullptr) ? beatPlayer->getDetectedKey().rootNote : -1;

    for (int i = 0; i < 12; ++i)
    {
        float barX = chromaArea.getX() + i * barWidth;
        float energy = std::clamp(smoothedChroma[i], 0.0f, 1.0f);
        float h = energy * maxHeight;
        float barY = chromaArea.getBottom() - 14.0f - h;

        bool isRoot = (i == activeRoot);

        // Bar Fill
        if (isRoot)
            g.setColour(juce::Colour(0xfff59e0b)); // Amber Root
        else
            g.setColour(juce::Colour(0xff0284c7).withAlpha(0.35f + energy * 0.65f));

        g.fillRoundedRectangle(barX + 1.0f, barY, barWidth - 2.0f, h, 2.0f);

        // Pitch Label text
        g.setColour(isRoot ? juce::Colour(0xfff59e0b) : juce::Colour(0xff64748b));
        g.setFont(juce::FontOptions(8.5f, isRoot ? juce::Font::bold : juce::Font::plain));
        g.drawText(pitchLabels[static_cast<size_t>(i)], static_cast<int>(barX), static_cast<int>(chromaArea.getBottom() - 12), static_cast<int>(barWidth), 12, juce::Justification::centred);
    }
}

void KeyDetectorComponent::updateDuckingButtonUI()
{
    if (beatPlayer == nullptr) return;
    const bool isDucking = beatPlayer->isDuckingEnabled();
    const float duckDb = beatPlayer->getDuckingAmountDb();

    if (isDucking)
    {
        duckingButton.setButtonText(juce::String("DUCK ") + juce::String(static_cast<int>(duckDb)) + "dB");
        duckingButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff059669)); // Emerald Green
        duckingButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    }
    else
    {
        duckingButton.setButtonText("DUCK: OFF");
        duckingButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff1e293b));
        duckingButton.setColour(juce::TextButton::textColourOffId, juce::Colour(0xff94a3b8));
    }
}

void KeyDetectorComponent::showDuckingSettingsMenu()
{
    if (beatPlayer == nullptr) return;

    juce::PopupMenu menu;
    const bool isDucking = beatPlayer->isDuckingEnabled();
    const float currentAmount = beatPlayer->getDuckingAmountDb();
    const float currentThresh = beatPlayer->getDuckingThresholdDb();
    const float currentHold = beatPlayer->getDuckingHoldMs();

    menu.addItem(100, juce::String::fromUTF8(u8"⚡ BẬT TÍNH NĂNG AUTO-DUCKING"), true, isDucking);
    menu.addSeparator();

    // 1. Mức giảm âm lượng (Ducking Depth)
    juce::PopupMenu depthMenu;
    depthMenu.addItem(1, juce::String::fromUTF8(u8"-6 dB (Hát đệm nhẹ nhàng)"), true, std::abs(currentAmount - (-6.0f)) < 0.5f);
    depthMenu.addItem(2, juce::String::fromUTF8(u8"-10 dB (Vừa phải / Hài hòa)"), true, std::abs(currentAmount - (-10.0f)) < 0.5f);
    depthMenu.addItem(3, juce::String::fromUTF8(u8"-12 dB (Tiêu chuẩn MC / Talkshow ⭐)"), true, std::abs(currentAmount - (-12.0f)) < 0.5f);
    depthMenu.addItem(4, juce::String::fromUTF8(u8"-15 dB (Giao lưu rõ ràng)"), true, std::abs(currentAmount - (-15.0f)) < 0.5f);
    depthMenu.addItem(5, juce::String::fromUTF8(u8"-20 dB (Chuyên Radio / Đọc truyện)"), true, std::abs(currentAmount - (-20.0f)) < 0.5f);
    depthMenu.addItem(6, juce::String::fromUTF8(u8"-30 dB (Hạ gần tắt hẳn)"), true, std::abs(currentAmount - (-30.0f)) < 0.5f);
    menu.addSubMenu(juce::String::fromUTF8(u8"📉 Mức giảm âm lượng beat (Depth)"), depthMenu);

    // 2. Độ nhạy bắt tiếng Micro (Threshold Sensitivity)
    juce::PopupMenu threshMenu;
    threshMenu.addItem(10, juce::String::fromUTF8(u8"-42 dB (Rất nhạy / Nói nhỏ, thầm)"), true, std::abs(currentThresh - (-42.0f)) < 1.0f);
    threshMenu.addItem(11, juce::String::fromUTF8(u8"-36 dB (Tiêu chuẩn Studio ⭐)"), true, std::abs(currentThresh - (-36.0f)) < 1.0f);
    threshMenu.addItem(12, juce::String::fromUTF8(u8"-28 dB (Ít nhạy / Phòng ồn, nói to)"), true, std::abs(currentThresh - (-28.0f)) < 1.0f);
    menu.addSubMenu(juce::String::fromUTF8(u8"🎙️ Độ nhạy bắt Micro (Sensitivity)"), threshMenu);

    // 3. Thời gian giữ tiếng sau khi ngừng nói (Hold Time)
    juce::PopupMenu holdMenu;
    holdMenu.addItem(20, juce::String::fromUTF8(u8"Nhanh (300 ms)"), true, std::abs(currentHold - 300.0f) < 50.0f);
    holdMenu.addItem(21, juce::String::fromUTF8(u8"Vừa phải (500 ms ⭐)"), true, std::abs(currentHold - 500.0f) < 50.0f);
    holdMenu.addItem(22, juce::String::fromUTF8(u8"Dài (800 ms - Không bị giật tiếng khi ngắt câu)"), true, std::abs(currentHold - 800.0f) < 50.0f);
    menu.addSubMenu(juce::String::fromUTF8(u8"⏱️ Thời gian giữ tiếng (Hold Time)"), holdMenu);

    menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(&duckingButton),
        [this](int result) {
            if (beatPlayer == nullptr || result == 0) return;

            if (result == 100)
            {
                beatPlayer->setDuckingEnabled(!beatPlayer->isDuckingEnabled());
            }
            else if (result == 1) { beatPlayer->setDuckingAmountDb(-6.0f); beatPlayer->setDuckingEnabled(true); }
            else if (result == 2) { beatPlayer->setDuckingAmountDb(-10.0f); beatPlayer->setDuckingEnabled(true); }
            else if (result == 3) { beatPlayer->setDuckingAmountDb(-12.0f); beatPlayer->setDuckingEnabled(true); }
            else if (result == 4) { beatPlayer->setDuckingAmountDb(-15.0f); beatPlayer->setDuckingEnabled(true); }
            else if (result == 5) { beatPlayer->setDuckingAmountDb(-20.0f); beatPlayer->setDuckingEnabled(true); }
            else if (result == 6) { beatPlayer->setDuckingAmountDb(-30.0f); beatPlayer->setDuckingEnabled(true); }
            else if (result == 10) beatPlayer->setDuckingThresholdDb(-42.0f);
            else if (result == 11) beatPlayer->setDuckingThresholdDb(-36.0f);
            else if (result == 12) beatPlayer->setDuckingThresholdDb(-28.0f);
            else if (result == 20) beatPlayer->setDuckingHoldMs(300.0f);
            else if (result == 21) beatPlayer->setDuckingHoldMs(500.0f);
            else if (result == 22) beatPlayer->setDuckingHoldMs(800.0f);

            updateDuckingButtonUI();
        });
}

void KeyDetectorComponent::updateAiShieldButtonUI()
{
#if HOSI_PRO_EDITION
    auto* dsp = graphManager.getBuiltInDsp();
    if (dsp == nullptr) return;

    const bool isAiOn = dsp->isAiDenoiseEnabled();
    const float redDb = dsp->getAiNoiseReductionDb();

    if (isAiOn)
    {
        if (redDb < -1.0f)
        {
            aiShieldButton.setButtonText(juce::String::fromUTF8(u8"🛡️ ") + juce::String(static_cast<int>(std::round(redDb))) + "dB");
        }
        else
        {
            aiShieldButton.setButtonText(juce::String::fromUTF8(u8"🛡️ AI ON"));
        }
        aiShieldButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff0284c7)); // Sky Blue / Cyan
        aiShieldButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    }
    else
    {
        aiShieldButton.setButtonText(juce::String::fromUTF8(u8"🛡️ AI: OFF"));
        aiShieldButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff1e293b));
        aiShieldButton.setColour(juce::TextButton::textColourOffId, juce::Colour(0xff94a3b8));
    }
#else
    aiShieldButton.setButtonText(juce::String::fromUTF8(u8"🔒 AI: PRO"));
    aiShieldButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff1e293b));
    aiShieldButton.setColour(juce::TextButton::textColourOffId, juce::Colour(0xfff59e0b)); // Amber/Gold
#endif
}

void KeyDetectorComponent::showAiShieldSettingsMenu()
{
#if !HOSI_PRO_EDITION
    juce::AlertWindow::showMessageBoxAsync(
        juce::AlertWindow::InfoIcon,
        juce::String::fromUTF8(u8"🔥 TÍNH NĂNG PRO EXCLUSIVE"),
        juce::String::fromUTF8(u8"Lá chắn AI Noise & Room De-Reverb Shield độc quyền trên LiveStream Micro-DAW PRO v3.0!\n\nHãy nâng cấp phiên bản PRO để sử dụng."),
        juce::String::fromUTF8(u8"Đã Hiểu")
    );
    return;
#else
    auto* dsp = graphManager.getBuiltInDsp();
    if (dsp == nullptr) return;

    juce::PopupMenu menu;
    const bool isDenoiseOn = dsp->isAiDenoiseEnabled();
    const bool isDeRevOn = dsp->isAiDeReverbEnabled();
    const float denoiseAmt = dsp->getAiDenoiseAmount();
    const float deRevAmt = dsp->getAiDeReverbAmount();

    menu.addSectionHeader(juce::String::fromUTF8(u8"🛡️ AI NOISE & DE-REVERB SHIELD (DEEPFILTER AI)"));
    menu.addItem(100, juce::String::fromUTF8(u8"⚡ BẬT / TẮT KHỬ ỒN AI (AI DENOISE)"), true, isDenoiseOn);
    menu.addItem(101, juce::String::fromUTF8(u8"🏛️ BẬT / TẮT TRIỆT TIÊU DỘI PHÒNG (AI DE-REVERB)"), true, isDeRevOn);
    menu.addSeparator();

    // 1. Mức Khử Ồn AI (Denoise Intensity)
    juce::PopupMenu denoiseMenu;
    denoiseMenu.addItem(1, juce::String::fromUTF8(u8"Nhẹ nhàng (40% - Phòng ít ồn, quạt xa)"), true, std::abs(denoiseAmt - 0.40f) < 0.08f);
    denoiseMenu.addItem(2, juce::String::fromUTF8(u8"Studio Tiêu Chuẩn (75% ⭐ - Khử quạt, ve kêu, giữ mượt giọng)"), true, std::abs(denoiseAmt - 0.75f) < 0.08f);
    denoiseMenu.addItem(3, juce::String::fromUTF8(u8"Triệt Để / Mạnh (90% - Phòng ồn nhiều, gần đường phố)"), true, std::abs(denoiseAmt - 0.90f) < 0.08f);
    denoiseMenu.addItem(4, juce::String::fromUTF8(u8"Tối đa (100% - Khử còi xe, bàn phím gõ mạnh)"), true, std::abs(denoiseAmt - 1.0f) < 0.05f);
    menu.addSubMenu(juce::String::fromUTF8(u8"🎚️ Cường độ khử ồn AI (Denoise Amount)"), denoiseMenu);

    // 2. Mức Triệt Tiêu Dội Phòng (De-Reverb Intensity)
    juce::PopupMenu deRevMenu;
    deRevMenu.addItem(10, juce::String::fromUTF8(u8"Nhẹ (30% - Phòng ngủ thông thường)"), true, std::abs(deRevAmt - 0.30f) < 0.08f);
    deRevMenu.addItem(11, juce::String::fromUTF8(u8"Vừa phải (55% ⭐ - Phòng trống chưa dán mút tiêu âm)"), true, std::abs(deRevAmt - 0.55f) < 0.08f);
    deRevMenu.addItem(12, juce::String::fromUTF8(u8"Mạnh (80% - Phòng khách / Hội trường dội nhiều)"), true, std::abs(deRevAmt - 0.80f) < 0.08f);
    menu.addSubMenu(juce::String::fromUTF8(u8"🏛️ Mức triệt tiêu dội phòng (Room De-Reverb)"), deRevMenu);

    menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(&aiShieldButton),
        [this, dsp](int result) {
            if (result == 0) return;

            if (result == 100)
            {
                dsp->setAiDenoiseEnabled(!dsp->isAiDenoiseEnabled());
            }
            else if (result == 101)
            {
                dsp->setAiDeReverbEnabled(!dsp->isAiDeReverbEnabled());
            }
            else if (result == 1) { dsp->setAiDenoiseAmount(0.40f); dsp->setAiDenoiseEnabled(true); }
            else if (result == 2) { dsp->setAiDenoiseAmount(0.75f); dsp->setAiDenoiseEnabled(true); }
            else if (result == 3) { dsp->setAiDenoiseAmount(0.90f); dsp->setAiDenoiseEnabled(true); }
            else if (result == 4) { dsp->setAiDenoiseAmount(1.00f); dsp->setAiDenoiseEnabled(true); }
            else if (result == 10) { dsp->setAiDeReverbAmount(0.30f); dsp->setAiDeReverbEnabled(true); }
            else if (result == 11) { dsp->setAiDeReverbAmount(0.55f); dsp->setAiDeReverbEnabled(true); }
            else if (result == 12) { dsp->setAiDeReverbAmount(0.80f); dsp->setAiDeReverbEnabled(true); }

            updateAiShieldButtonUI();
        });
#endif
}

void KeyDetectorComponent::resized()
{
    auto area = getLocalBounds().reduced(8, 4);

    // Reserve right side for Chroma Visualizer
    area.removeFromRight(150);

    // Middle-Right Section: Key Detection & Tempo Controls (~330px)
    auto keySection = area.removeFromRight(330);

    // Top sub-row in key section: Title, Confidence, Auto-Push, and Tap Tempo
    keyTitleLabel.setBounds(keySection.getX(), area.getY() + 2, 80, 14);
    confidenceLabel.setBounds(keySection.getX() + 82, area.getY() + 2, 110, 14);
    autoPushToggle.setBounds(keySection.getX() + 194, area.getY() + 1, 74, 16);
    tapTempoButton.setBounds(keySection.getX() + 272, area.getY() + 1, 56, 16);

    // Middle sub-row in key section: Display Label + BPM Button + Quick Manual Select Button
    keyDisplayLabel.setBounds(keySection.getX(), area.getY() + 18, 140, 22);
    bpmButton.setBounds(keySection.getX() + 144, area.getY() + 18, 86, 22);
    manualKeyButton.setBounds(keySection.getX() + 234, area.getY() + 18, 94, 22);

    // Bottom sub-row in key section: Sync Button + Source Toggle Button
    syncToAutoTuneButton.setBounds(keySection.getX(), area.getY() + 42, 190, 24);
    sourceToggleButton.setBounds(keySection.getX() + 196, area.getY() + 42, 132, 24);

    // Left Section: Beat Player Controls
    auto playerSection = area;
    auto topRow = playerSection.removeFromTop(28);
    auto bottomRow = playerSection;

    // Top Row: Load Button, Play/Pause Icon (34px), Stop Icon (32px), Loop Icon (32px), File Time
    loadBeatButton.setBounds(topRow.removeFromLeft(76).reduced(0, 2));
    topRow.removeFromLeft(4);
    playPauseButton.setBounds(topRow.removeFromLeft(36).reduced(0, 2));
    topRow.removeFromLeft(3);
    stopButton.setBounds(topRow.removeFromLeft(32).reduced(0, 2));
    topRow.removeFromLeft(3);
    loopButton.setBounds(topRow.removeFromLeft(32).reduced(0, 2));
    topRow.removeFromLeft(6);
    timeLabel.setBounds(topRow.removeFromLeft(78));

    // Top Row Volume
    volumeLabel.setBounds(topRow.removeFromLeft(24));
    volumeSlider.setBounds(topRow.removeFromLeft(56).reduced(0, 4));

    // Top Row Smart Ducking Button
    topRow.removeFromLeft(5);
    duckingButton.setBounds(topRow.removeFromLeft(86).reduced(0, 2));

    // Top Row AI Shield Button
    topRow.removeFromLeft(4);
    aiShieldButton.setBounds(topRow.removeFromLeft(88).reduced(0, 2));

    // Top Row Quick Record & Folder Buttons
    topRow.removeFromLeft(4);
    recButton.setBounds(topRow.removeFromLeft(64).reduced(0, 2));
    topRow.removeFromLeft(3);
    recFolderButton.setBounds(topRow.removeFromLeft(28).reduced(0, 2));

    // Bottom Row: Position Seek Slider
    positionSlider.setBounds(bottomRow.reduced(2, 2));
}
