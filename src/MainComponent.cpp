#include "MainComponent.h"

MainComponent::MainComponent()
    : presetManager(audioEngine.getGraphManager())
{
    // Initialize Audio Engine first
    audioEngine.initialize();

    // Header title
    titleLabel.setText("LIVESTREAM MICRO-DAW", juce::dontSendNotification);
    titleLabel.setFont(juce::FontOptions(15.0f, juce::Font::bold));
    titleLabel.setColour(juce::Label::textColourId, juce::Colour(0xffffffff));
    addAndMakeVisible(titleLabel);

    // Audio API Mode badge
    modeBadgeLabel.setFont(juce::FontOptions(11.0f, juce::Font::bold));
    modeBadgeLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(modeBadgeLabel);

    // Mic Mute Master button
    muteMicButton.onClick = [this]() {
        const bool newMute = !audioEngine.getGraphManager().isMasterMuted();
        audioEngine.getGraphManager().setMasterMute(newMute);
        updateMuteButtonUI();
    };
    addAndMakeVisible(muteMicButton);

    // Input Source Routing Button (Mic 1 / Mic 2 / Stereo)
    inputSourceButton.onClick = [this]() {
        const auto currentMode = audioEngine.getGraphManager().getInputSourceMode();
        int nextMode = (static_cast<int>(currentMode) + 1) % 3;
        audioEngine.getGraphManager().setInputSourceMode(static_cast<InputRouterAudioProcessor::InputMode>(nextMode));
        updateInputSourceButtonUI();
    };
    addAndMakeVisible(inputSourceButton);

    // Stats / Latency readout
    statsLabel.setFont(juce::FontOptions(11.0f, juce::Font::plain));
    statsLabel.setColour(juce::Label::textColourId, juce::Colour(0xff94a3b8));
    statsLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(statsLabel);

    // Quick Scene buttons (F1, F2, F3)
    liveSingingPresetBtn.setTooltip(juce::String::fromUTF8(u8"Chế độ Hát Live: Bật trọn bộ Vocal Effect (Phím tắt: F1)"));
    liveSingingPresetBtn.onClick = [this]() {
        presetManager.applyQuickPreset(PresetManager::QuickPresetType::LiveSinging);
        updatePresetButtonsUI();
    };
    addAndMakeVisible(liveSingingPresetBtn);

    talkPresetBtn.setTooltip(juce::String::fromUTF8(u8"Chế độ Giao Lưu: Tắt Vang/Tune, giọng mộc rõ tiếng (Phím tắt: F2)"));
    talkPresetBtn.onClick = [this]() {
        presetManager.toggleTalkMode();
        updatePresetButtonsUI();
    };
    addAndMakeVisible(talkPresetBtn);

    trapPresetBtn.setTooltip(juce::String::fromUTF8(u8"Chế độ AutoTune Remix / Trap sôi động (Phím tắt: F3)"));
    trapPresetBtn.onClick = [this]() {
        presetManager.applyQuickPreset(PresetManager::QuickPresetType::HighEnergyTrap);
        updatePresetButtonsUI();
    };
    addAndMakeVisible(trapPresetBtn);

    // Custom Preset File Management (Save / Load)
    savePresetBtn.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff164e63));
    savePresetBtn.setColour(juce::TextButton::textColourOffId, juce::Colour(0xff67e8f9));
    savePresetBtn.onClick = [this]() { handleSavePreset(); };
    addAndMakeVisible(savePresetBtn);

    loadPresetBtn.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff1e293b));
    loadPresetBtn.setColour(juce::TextButton::textColourOffId, juce::Colour(0xffcbd5e1));
    loadPresetBtn.onClick = [this]() { handleLoadPreset(); };
    addAndMakeVisible(loadPresetBtn);

    // Streamer Floating Mini-Bar Mode (Crisp Vector Icon Button)
    compactModeButton.setTooltip(juce::String::fromUTF8(u8"Thu gọn thành Mini Bar / Mở rộng Full Studio Rack"));
    compactModeButton.onClick = [this]() { toggleCompactMode(); };
    addAndMakeVisible(compactModeButton);

    // Pin Always on Top
    alwaysOnTopButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff1e293b));
    alwaysOnTopButton.setColour(juce::TextButton::textColourOffId, juce::Colour(0xff94a3b8));
    alwaysOnTopButton.setTooltip(juce::String::fromUTF8(u8"Ghim cửa sổ luôn nổi trên cùng (Always On Top)"));
    alwaysOnTopButton.onClick = [this]() { toggleAlwaysOnTop(); };
    addAndMakeVisible(alwaysOnTopButton);

    // Audio settings button
    settingsButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff1e293b));
    settingsButton.setColour(juce::TextButton::textColourOffId, juce::Colour(0xff38bdf8));
    settingsButton.onClick = [this]() {
        showSettings(!isSettingsOverlayVisible);
    };
    addAndMakeVisible(settingsButton);

    // Songbook & Tone Manager button
    songbookButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff065f46)); // Emerald green
    songbookButton.setColour(juce::TextButton::textColourOffId, juce::Colour(0xff6ee7b7)); // Mint
    songbookButton.setTooltip(juce::String::fromUTF8(u8"Mở Sổ Tone Bài Hát (Songbook & Auto-Key) - Tra cứu tone và nạp vào Auto-Tune"));
    songbookButton.onClick = [this]() {
        showSongbook(!isSongbookOverlayVisible);
    };
    addAndMakeVisible(songbookButton);

#if HOSI_PRO_EDITION
    // YouTube Karaoke Player Button (PRO Edition)
    youtubeButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff991b1b)); // Crimson red
    youtubeButton.setColour(juce::TextButton::textColourOffId, juce::Colour(0xfffecdd3));
    youtubeButton.setTooltip(juce::String::fromUTF8(u8"Mở Mini YouTube Karaoke Player (Tự động bỏ qua quảng cáo, Chế độ Live)"));
    youtubeButton.onClick = [this]() {
        showYouTubePlayer(!isYouTubeOverlayVisible);
    };
    addAndMakeVisible(youtubeButton);
#endif

    // Donate / Support Creator button
    donateButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff7c2d12)); // Warm amber/orange
    donateButton.setColour(juce::TextButton::textColourOffId, juce::Colour(0xfffcd34d)); // Gold
    donateButton.setTooltip(juce::String::fromUTF8(u8"Ủng hộ tác giả phát triển phần mềm (Mã QR MoMo / MBBank)"));
    donateButton.onClick = [this]() {
        showDonate(!isDonateOverlayVisible);
    };
    addAndMakeVisible(donateButton);

    // Update Checker Button
    updateCheckButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff1e293b));
    updateCheckButton.setColour(juce::TextButton::textColourOffId, juce::Colour(0xff38bdf8));
    updateCheckButton.setTooltip(juce::String::fromUTF8(u8"Kiểm tra và cập nhật phiên bản mới nhất từ GitHub"));
    updateCheckButton.onClick = [this]() {
        UpdateChecker::check(true); // Manual check
    };
    addAndMakeVisible(updateCheckButton);

    // Auto-check for updates 3.5s after app startup (silent in background)
    juce::Timer::callAfterDelay(3500, [] {
        UpdateChecker::check(false);
    });

    // Real-time Beat Player & Auto Key Detector Bar
    keyDetectorBar = std::make_unique<KeyDetectorComponent>(audioEngine.getGraphManager());
    addAndMakeVisible(keyDetectorBar.get());

    // Vertical Rack Component
    verticalRack = std::make_unique<VerticalRackComponent>(audioEngine.getGraphManager());
    addAndMakeVisible(verticalRack.get());

    // Right Column: Tab Switch Buttons
    masterTabButton.setButtonText("MASTER");
    masterTabButton.onClick = [this] { setRightTab(RightTab::Master); };
    addAndMakeVisible(masterTabButton);

    vocalDspTabButton.setButtonText("VOCAL DSP");
    vocalDspTabButton.onClick = [this] { setRightTab(RightTab::VocalDsp); };
    addAndMakeVisible(vocalDspTabButton);

    soundboardTabButton.setButtonText("SOUND FX");
    soundboardTabButton.onClick = [this] { setRightTab(RightTab::Soundboard); };
    addAndMakeVisible(soundboardTabButton);

    // Master Output Strip (Fader + Dual Stereo Peak Meter + Master Limiter)
    masterStrip = std::make_unique<MasterChannelStripComponent>(audioEngine.getGraphManager());
    addAndMakeVisible(masterStrip.get());

    // Built-in Studio Vocal DSP Panel
    builtInDspPanel = std::make_unique<BuiltInDspComponent>(audioEngine.getGraphManager());
    builtInDspPanel->setVisible(false);
    addChildComponent(builtInDspPanel.get());

    // Soundboard Panel
    soundboardPanel = std::make_unique<SoundboardComponent>(audioEngine.getGraphManager());
    soundboardPanel->setVisible(false);
    addChildComponent(soundboardPanel.get());

    updateRightTabButtonsUI();

    // Enable Keyboard focus for Soundboard Hotkeys (Numpad 1..8)
    setWantsKeyboardFocus(true);

    // Settings Overlay
    settingsOverlay = std::make_unique<AudioSettingsOverlay>(audioEngine);
    settingsOverlay->onCloseClicked = [this]() {
        showSettings(false);
    };
    settingsOverlay->onUiScaleChanged = [this](float scale, int w, int h) {
        applyUiScale(scale, w, h);
    };
    addChildComponent(settingsOverlay.get());

    // Donate Overlay
    donateOverlay = std::make_unique<DonateOverlay>();
    donateOverlay->onCloseClicked = [this]() {
        showDonate(false);
    };
    addChildComponent(donateOverlay.get());

    // Songbook Overlay & AI Vocal Range
    auto* dsp = audioEngine.getGraphManager().getBuiltInDsp();
    jassert(dsp != nullptr);
    auto& vocalDetector = dsp->getVocalRangeDetector();
    songbookOverlay = std::make_unique<SongbookOverlay>(songbookManager, vocalDetector);

    songbookOverlay->onCloseClicked = [this]() {
        showSongbook(false);
    };
    songbookOverlay->onOpenVocalRangeDetector = [this]() {
        showVocalRangeOverlay(true);
    };
    songbookOverlay->onApplyTone = [this](int rootNote, KeyDetector::ScaleType scale, const juce::String& songName) {
        if (keyDetectorBar != nullptr)
        {
            keyDetectorBar->applyKeyToAutoTune(rootNote, scale, songName);
        }
    };
    songbookOverlay->onApplyTempo = [this](double bpm, const juce::String& songName) {
        audioEngine.getGraphManager().getTempoSyncEngine().setBpm(bpm, songName);
    };
#if HOSI_PRO_EDITION
    songbookOverlay->onPlayYouTubeBeat = [this](const juce::String& songName) {
        showYouTubePlayer(true, songName);
    };
#endif
    addChildComponent(songbookOverlay.get());

    // AI Vocal Range & Song Recommendation Overlay
    vocalRangeOverlay = std::make_unique<AiVocalRangeOverlay>(vocalDetector, songbookManager);
    vocalRangeOverlay->onCloseClicked = [this]() {
        showVocalRangeOverlay(false);
    };
    vocalRangeOverlay->onApplyTone = [this](int rootNote, KeyDetector::ScaleType scale, const juce::String& songName) {
        if (keyDetectorBar != nullptr)
        {
            keyDetectorBar->applyKeyToAutoTune(rootNote, scale, songName);
        }
    };
    vocalRangeOverlay->onApplyTempo = [this](double bpm, const juce::String& songName) {
        audioEngine.getGraphManager().getTempoSyncEngine().setBpm(bpm, songName);
    };
#if HOSI_PRO_EDITION
    vocalRangeOverlay->onPlayYouTubeBeat = [this](const juce::String& songName) {
        showYouTubePlayer(true, songName);
    };
#endif
    vocalRangeOverlay->onOpenFullSongbookAi = [this]() {
        showVocalRangeOverlay(false);
        showSongbook(true);
        if (songbookOverlay != nullptr)
        {
            songbookOverlay->setFilterToAiMatch();
        }
    };
    addChildComponent(vocalRangeOverlay.get());


#if HOSI_PRO_EDITION
    // YouTube Karaoke Player Overlay (PRO Edition)
    youtubeOverlay = std::make_unique<YouTubePlayerOverlay>(&songbookManager);
    youtubeOverlay->onCloseClicked = [this]() {
        showYouTubePlayer(false);
    };
    youtubeOverlay->onDetectAndPushToAutoTune = [this]() {
        if (keyDetectorBar != nullptr)
        {
            keyDetectorBar->syncKeyToPitchPlugin();
        }
    };
    youtubeOverlay->onApplyTone = [this](int rootNote, KeyDetector::ScaleType scale, const juce::String& songName, bool showNotificationPopup) {
        if (keyDetectorBar != nullptr)
        {
            keyDetectorBar->applyKeyToAutoTune(rootNote, scale, songName, showNotificationPopup);
        }
    };
    youtubeOverlay->onApplyTempo = [this](double bpm, const juce::String& songName) {
        audioEngine.getGraphManager().getTempoSyncEngine().setBpm(bpm, songName);
    };
    youtubeOverlay->onOpenSongbook = [this]() {
        showSongbook(true);
    };
    addChildComponent(youtubeOverlay.get());
#endif

    // Restore persistent session state (plugins, custom slots, gain levels, bypass, window size, ui scale)
    presetManager.restoreSessionState();

    previousFullWidth = std::clamp(presetManager.getSavedWindowWidth(), 700, 1920);
    previousFullHeight = std::clamp(presetManager.getSavedWindowHeight(), 500, 1200);

    const float savedScale = presetManager.getSavedUiScale();
    if (savedScale >= 0.70f && savedScale <= 1.60f)
    {
        juce::Desktop::getInstance().setGlobalScaleFactor(savedScale);
    }

    if (verticalRack != nullptr)
        verticalRack->rebuildSlotUI();

    if (masterStrip != nullptr)
        masterStrip->updateAllUI();

    updateMuteButtonUI();
    updateInputSourceButtonUI();
    updatePresetButtonsUI();

    // Set initial size and layout AFTER all child components are ready
    setSize(previousFullWidth, previousFullHeight);

    startTimerHz(30);
}

MainComponent::~MainComponent()
{
    stopTimer();
    presetManager.setWindowSize(previousFullWidth, previousFullHeight);
    presetManager.saveSessionState();
    audioEngine.saveDeviceState();
    audioEngine.shutdown();
}

void MainComponent::updateInputSourceButtonUI()
{
    const auto mode = audioEngine.getGraphManager().getInputSourceMode();
    if (mode == InputRouterAudioProcessor::InputMode::MonoIn1)
    {
        inputSourceButton.setButtonText("IN: MIC 1 (L+R)");
        inputSourceButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff0f3044));
        inputSourceButton.setColour(juce::TextButton::textColourOffId, juce::Colour(0xff38bdf8));
    }
    else if (mode == InputRouterAudioProcessor::InputMode::MonoIn2)
    {
        inputSourceButton.setButtonText("IN: MIC 2 (L+R)");
        inputSourceButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff3b1a45));
        inputSourceButton.setColour(juce::TextButton::textColourOffId, juce::Colour(0xffe879f9));
    }
    else
    {
        inputSourceButton.setButtonText("IN: STEREO");
        inputSourceButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff1e293b));
        inputSourceButton.setColour(juce::TextButton::textColourOffId, juce::Colour(0xff94a3b8));
    }
    repaint();
}

void MainComponent::updateMuteButtonUI()
{
    const bool isMuted = audioEngine.getGraphManager().isMasterMuted();
    if (isMuted)
    {
        muteMicButton.setButtonText("MIC MUTED");
        muteMicButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xffdc2626)); // Red
        muteMicButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    }
    else
    {
        muteMicButton.setButtonText("MIC ON");
        muteMicButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff047857)); // Green
        muteMicButton.setColour(juce::TextButton::textColourOffId, juce::Colour(0xffa7f3d0));
    }
    repaint();
}

void MainComponent::updatePresetButtonsUI()
{
    const bool isTalk = presetManager.isTalkModeActive();
    const auto currentPreset = presetManager.getCurrentPreset();

    if (isTalk)
    {
        // TALK CHAT is Active ON (Bright Blue Glow)
        talkPresetBtn.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff2563eb));
        talkPresetBtn.setColour(juce::TextButton::textColourOffId, juce::Colour(0xffffffff));

        // Other buttons dimmed
        liveSingingPresetBtn.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff132e27));
        liveSingingPresetBtn.setColour(juce::TextButton::textColourOffId, juce::Colour(0xff4ade80));

        trapPresetBtn.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff331338));
        trapPresetBtn.setColour(juce::TextButton::textColourOffId, juce::Colour(0xffe879f9));
    }
    else
    {
        // TALK CHAT is OFF (Idle Dark Slate)
        talkPresetBtn.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff1e293b));
        talkPresetBtn.setColour(juce::TextButton::textColourOffId, juce::Colour(0xff94a3b8));

        if (currentPreset == PresetManager::QuickPresetType::LiveSinging)
        {
            // LIVE VOCAL is Active (Vibrant Green)
            liveSingingPresetBtn.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff059669));
            liveSingingPresetBtn.setColour(juce::TextButton::textColourOffId, juce::Colour(0xffffffff));

            trapPresetBtn.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff331338));
            trapPresetBtn.setColour(juce::TextButton::textColourOffId, juce::Colour(0xffe879f9));
        }
        else if (currentPreset == PresetManager::QuickPresetType::HighEnergyTrap)
        {
            // AUTOTUNE is Active (Vibrant Purple)
            trapPresetBtn.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff9333ea));
            trapPresetBtn.setColour(juce::TextButton::textColourOffId, juce::Colour(0xffffffff));

            liveSingingPresetBtn.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff132e27));
            liveSingingPresetBtn.setColour(juce::TextButton::textColourOffId, juce::Colour(0xff4ade80));
        }
    }

    repaint();
}

void MainComponent::showSettings(bool show)
{
    if (show && isCompactMode)
    {
        wasInCompactModeBeforeOverlay = true;
        toggleCompactMode();
    }

    isSettingsOverlayVisible = show;
    if (settingsOverlay != nullptr)
    {
        settingsOverlay->setVisible(show);
        if (show)
        {
            if (isDonateOverlayVisible)
            {
                isDonateOverlayVisible = false;
                if (donateOverlay != nullptr) donateOverlay->setVisible(false);
            }
            if (isSongbookOverlayVisible)
            {
                isSongbookOverlayVisible = false;
                if (songbookOverlay != nullptr) songbookOverlay->setVisible(false);
            }
#if HOSI_PRO_EDITION
            if (isYouTubeOverlayVisible)
            {
                isYouTubeOverlayVisible = false;
                if (youtubeOverlay != nullptr) youtubeOverlay->setVisible(false);
            }
#endif
            settingsOverlay->updateScaleButtonsUI(presetManager.getSavedUiScale());
            settingsOverlay->toFront(true);
        }
    }

    bool anyOverlayStillOpen = isDonateOverlayVisible || isSongbookOverlayVisible;
#if HOSI_PRO_EDITION
    anyOverlayStillOpen = anyOverlayStillOpen || isYouTubeOverlayVisible;
#endif

    if (!show && wasInCompactModeBeforeOverlay && !anyOverlayStillOpen)
    {
        wasInCompactModeBeforeOverlay = false;
        if (!isCompactMode)
            toggleCompactMode();
    }

    resized();
}

void MainComponent::showDonate(bool show)
{
    if (show && isCompactMode)
    {
        wasInCompactModeBeforeOverlay = true;
        toggleCompactMode();
    }

    isDonateOverlayVisible = show;
    if (donateOverlay != nullptr)
    {
        donateOverlay->setVisible(show);
        if (show)
        {
            if (isSettingsOverlayVisible)
            {
                isSettingsOverlayVisible = false;
                if (settingsOverlay != nullptr) settingsOverlay->setVisible(false);
            }
            if (isSongbookOverlayVisible)
            {
                isSongbookOverlayVisible = false;
                if (songbookOverlay != nullptr) songbookOverlay->setVisible(false);
            }
#if HOSI_PRO_EDITION
            if (isYouTubeOverlayVisible)
            {
                isYouTubeOverlayVisible = false;
                if (youtubeOverlay != nullptr) youtubeOverlay->setVisible(false);
            }
#endif
            donateOverlay->toFront(true);
        }
    }

    bool anyOverlayStillOpen = isSettingsOverlayVisible || isSongbookOverlayVisible;
#if HOSI_PRO_EDITION
    anyOverlayStillOpen = anyOverlayStillOpen || isYouTubeOverlayVisible;
#endif

    if (!show && wasInCompactModeBeforeOverlay && !anyOverlayStillOpen)
    {
        wasInCompactModeBeforeOverlay = false;
        if (!isCompactMode)
            toggleCompactMode();
    }

    resized();
}

void MainComponent::showSongbook(bool show)
{
    if (show && isCompactMode)
    {
        wasInCompactModeBeforeOverlay = true;
        toggleCompactMode();
    }

    isSongbookOverlayVisible = show;
    if (songbookOverlay != nullptr)
    {
        songbookOverlay->setVisible(show);
        if (show)
        {
            if (isSettingsOverlayVisible)
            {
                isSettingsOverlayVisible = false;
                if (settingsOverlay != nullptr) settingsOverlay->setVisible(false);
            }
            if (isDonateOverlayVisible)
            {
                isDonateOverlayVisible = false;
                if (donateOverlay != nullptr) donateOverlay->setVisible(false);
            }
#if HOSI_PRO_EDITION
            if (isYouTubeOverlayVisible)
            {
                isYouTubeOverlayVisible = false;
                if (youtubeOverlay != nullptr) youtubeOverlay->setVisible(false);
            }
#endif
            songbookOverlay->toFront(true);
        }
    }

    bool anyOverlayStillOpen = isSettingsOverlayVisible || isDonateOverlayVisible || isVocalRangeOverlayVisible;
#if HOSI_PRO_EDITION
    anyOverlayStillOpen = anyOverlayStillOpen || isYouTubeOverlayVisible;
#endif

    if (!show && wasInCompactModeBeforeOverlay && !anyOverlayStillOpen)
    {
        wasInCompactModeBeforeOverlay = false;
        if (!isCompactMode)
            toggleCompactMode();
    }

    resized();
}

void MainComponent::showVocalRangeOverlay(bool show)
{
    if (show && isCompactMode)
    {
        wasInCompactModeBeforeOverlay = true;
        toggleCompactMode();
    }

    isVocalRangeOverlayVisible = show;
    if (vocalRangeOverlay != nullptr)
    {
        vocalRangeOverlay->setVisible(show);
        if (show)
        {
            if (isSettingsOverlayVisible)
            {
                isSettingsOverlayVisible = false;
                if (settingsOverlay != nullptr) settingsOverlay->setVisible(false);
            }
            if (isDonateOverlayVisible)
            {
                isDonateOverlayVisible = false;
                if (donateOverlay != nullptr) donateOverlay->setVisible(false);
            }
            if (isSongbookOverlayVisible)
            {
                isSongbookOverlayVisible = false;
                if (songbookOverlay != nullptr) songbookOverlay->setVisible(false);
            }
#if HOSI_PRO_EDITION
            if (isYouTubeOverlayVisible)
            {
                isYouTubeOverlayVisible = false;
                if (youtubeOverlay != nullptr) youtubeOverlay->setVisible(false);
            }
#endif
            vocalRangeOverlay->toFront(true);
        }
    }

    bool anyOverlayStillOpen = isSettingsOverlayVisible || isDonateOverlayVisible || isSongbookOverlayVisible;
#if HOSI_PRO_EDITION
    anyOverlayStillOpen = anyOverlayStillOpen || isYouTubeOverlayVisible;
#endif

    if (!show && wasInCompactModeBeforeOverlay && !anyOverlayStillOpen)
    {
        wasInCompactModeBeforeOverlay = false;
        if (!isCompactMode)
            toggleCompactMode();
    }

    resized();
}


#if HOSI_PRO_EDITION
void MainComponent::showYouTubePlayer(bool show, const juce::String& initialSongName)
{
    if (show && isCompactMode)
    {
        wasInCompactModeBeforeOverlay = true;
        toggleCompactMode();
    }

    isYouTubeOverlayVisible = show;
    if (youtubeOverlay != nullptr)
    {
        youtubeOverlay->setVisible(show);
        if (show)
        {
            if (isSettingsOverlayVisible)
            {
                isSettingsOverlayVisible = false;
                if (settingsOverlay != nullptr) settingsOverlay->setVisible(false);
            }
            if (isDonateOverlayVisible)
            {
                isDonateOverlayVisible = false;
                if (donateOverlay != nullptr) donateOverlay->setVisible(false);
            }
            if (isSongbookOverlayVisible)
            {
                isSongbookOverlayVisible = false;
                if (songbookOverlay != nullptr) songbookOverlay->setVisible(false);
            }
            
            if (initialSongName.isNotEmpty())
            {
                youtubeOverlay->searchAndPlay(initialSongName);
            }

            youtubeOverlay->toFront(true);
        }
    }

    bool anyOverlayStillOpen = isSettingsOverlayVisible || isDonateOverlayVisible || isSongbookOverlayVisible;

    if (!show && wasInCompactModeBeforeOverlay && !anyOverlayStillOpen)
    {
        wasInCompactModeBeforeOverlay = false;
        if (!isCompactMode)
            toggleCompactMode();
    }

    resized();
}
#endif

void MainComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff0b0d13));

    // Header bar background
    const int headerH = isCompactMode ? getHeight() : 62;
    auto headerBounds = getLocalBounds().removeFromTop(headerH).toFloat();
    g.setColour(juce::Colour(0xff12151f));
    g.fillRect(headerBounds);

    // Bottom border for header
    if (!isCompactMode)
    {
        g.setColour(juce::Colour(0xff1e2433));
        g.drawHorizontalLine(headerH, 0.0f, static_cast<float>(getWidth()));
    }
}

void MainComponent::resized()
{
    auto bounds = getLocalBounds();

    // Top Header area (62px)
    const int headerH = isCompactMode ? bounds.getHeight() : 62;
    auto headerArea = bounds.removeFromTop(headerH).reduced(6, 10);
    
    // Left Branding & Routing
    titleLabel.setBounds(headerArea.removeFromLeft(135));
    modeBadgeLabel.setBounds(headerArea.removeFromLeft(135).reduced(2));
    inputSourceButton.setBounds(headerArea.removeFromLeft(92).reduced(2));
    muteMicButton.setBounds(headerArea.removeFromLeft(64).reduced(2));

    // Right-side Utility & Window control buttons
    compactModeButton.setBounds(headerArea.removeFromRight(38).reduced(2));
    alwaysOnTopButton.setBounds(headerArea.removeFromRight(42).reduced(2));
    settingsButton.setBounds(headerArea.removeFromRight(76).reduced(2));
    updateCheckButton.setBounds(headerArea.removeFromRight(66).reduced(2));
    donateButton.setBounds(headerArea.removeFromRight(78).reduced(2));
    songbookButton.setBounds(headerArea.removeFromRight(84).reduced(2));
#if HOSI_PRO_EDITION
    youtubeButton.setBounds(headerArea.removeFromRight(112).reduced(2));
#endif
    
    // Quick Preset Scene buttons
    liveSingingPresetBtn.setBounds(headerArea.removeFromLeft(76).reduced(2));
    talkPresetBtn.setBounds(headerArea.removeFromLeft(50).reduced(2));
    trapPresetBtn.setBounds(headerArea.removeFromLeft(86).reduced(2));

    // Custom Preset Management (Save / Load)
    savePresetBtn.setBounds(headerArea.removeFromLeft(50).reduced(2));
    loadPresetBtn.setBounds(headerArea.removeFromLeft(50).reduced(2));

    // Center / Flexible space for stats readout
    statsLabel.setBounds(headerArea.reduced(4, 0));

    if (!isCompactMode)
    {
        if (keyDetectorBar != nullptr)
        {
            keyDetectorBar->setVisible(true);
            keyDetectorBar->setBounds(bounds.removeFromTop(74).reduced(6, 3));
        }

        // Right Column: Master Strip / Built-In DSP / Soundboard (204px)
        auto rightArea = bounds.removeFromRight(204).reduced(4, 4);

        // Tab switch header bar (24px)
        auto tabArea = rightArea.removeFromTop(24);
        masterTabButton.setVisible(true);
        vocalDspTabButton.setVisible(true);
        soundboardTabButton.setVisible(true);

        const int tabW = (tabArea.getWidth() - 4) / 3;
        masterTabButton.setBounds(tabArea.getX(), tabArea.getY(), tabW, tabArea.getHeight());
        vocalDspTabButton.setBounds(tabArea.getX() + tabW + 2, tabArea.getY(), tabW, tabArea.getHeight());
        soundboardTabButton.setBounds(tabArea.getX() + 2 * (tabW + 2), tabArea.getY(), tabArea.getRight() - (tabArea.getX() + 2 * (tabW + 2)), tabArea.getHeight());

        rightArea.removeFromTop(4);

        if (masterStrip != nullptr)
        {
            masterStrip->setVisible(currentRightTab == RightTab::Master);
            if (currentRightTab == RightTab::Master) masterStrip->setBounds(rightArea);
        }
        if (builtInDspPanel != nullptr)
        {
            builtInDspPanel->setVisible(currentRightTab == RightTab::VocalDsp);
            if (currentRightTab == RightTab::VocalDsp) builtInDspPanel->setBounds(rightArea);
        }
        if (soundboardPanel != nullptr)
        {
            soundboardPanel->setVisible(currentRightTab == RightTab::Soundboard);
            if (currentRightTab == RightTab::Soundboard) soundboardPanel->setBounds(rightArea);
        }

        // Left Rack area
        if (verticalRack != nullptr)
        {
            verticalRack->setBounds(bounds);
        }
    }
    else
    {
        if (keyDetectorBar != nullptr)
            keyDetectorBar->setVisible(false);
        masterTabButton.setVisible(false);
        vocalDspTabButton.setVisible(false);
        soundboardTabButton.setVisible(false);
        if (masterStrip != nullptr)
            masterStrip->setVisible(false);
        if (builtInDspPanel != nullptr)
            builtInDspPanel->setVisible(false);
        if (soundboardPanel != nullptr)
            soundboardPanel->setVisible(false);
    }

    // Overlay areas
    if (settingsOverlay != nullptr && settingsOverlay->isVisible())
    {
        settingsOverlay->setBounds(getLocalBounds());
    }
    if (donateOverlay != nullptr && donateOverlay->isVisible())
    {
        donateOverlay->setBounds(getLocalBounds());
    }
    if (songbookOverlay != nullptr && songbookOverlay->isVisible())
    {
        songbookOverlay->setBounds(getLocalBounds());
    }
    if (vocalRangeOverlay != nullptr && vocalRangeOverlay->isVisible())
    {
        vocalRangeOverlay->setBounds(getLocalBounds());
    }
#if HOSI_PRO_EDITION
    if (youtubeOverlay != nullptr && youtubeOverlay->isVisible())
    {
        youtubeOverlay->setBounds(getLocalBounds());
    }
#endif
}


void MainComponent::setRightTab(RightTab tab)
{
    currentRightTab = tab;
    if (masterStrip != nullptr)
        masterStrip->setVisible(tab == RightTab::Master);
    if (builtInDspPanel != nullptr)
        builtInDspPanel->setVisible(tab == RightTab::VocalDsp);
    if (soundboardPanel != nullptr)
        soundboardPanel->setVisible(tab == RightTab::Soundboard);

    updateRightTabButtonsUI();
    resized();
}

void MainComponent::updateRightTabButtonsUI()
{
    auto styleTab = [](juce::TextButton& btn, bool active, juce::Colour activeBg, juce::Colour activeText)
    {
        btn.setColour(juce::TextButton::buttonColourId, active ? activeBg : juce::Colour(0xff1e293b));
        btn.setColour(juce::TextButton::textColourOffId, active ? activeText : juce::Colour(0xff94a3b8));
    };

    styleTab(masterTabButton, currentRightTab == RightTab::Master, juce::Colour(0xff0284c7), juce::Colours::white);
    styleTab(vocalDspTabButton, currentRightTab == RightTab::VocalDsp, juce::Colour(0xff059669), juce::Colours::white); // Emerald Green
    styleTab(soundboardTabButton, currentRightTab == RightTab::Soundboard, juce::Colour(0xfff59e0b), juce::Colour(0xff0f172a)); // Gold
}

bool MainComponent::keyPressed(const juce::KeyPress& key)
{
    const auto keyCode = key.getKeyCode();

    // Hotkeys F1, F2, F3 for Scenes
    if (keyCode == juce::KeyPress::F1Key)
    {
        presetManager.applyQuickPreset(PresetManager::QuickPresetType::LiveSinging);
        updatePresetButtonsUI();
        return true;
    }
    else if (keyCode == juce::KeyPress::F2Key)
    {
        presetManager.toggleTalkMode();
        updatePresetButtonsUI();
        return true;
    }
    else if (keyCode == juce::KeyPress::F3Key)
    {
        presetManager.applyQuickPreset(PresetManager::QuickPresetType::HighEnergyTrap);
        updatePresetButtonsUI();
        return true;
    }

    // Hotkeys 1..8 (and Numpad 1..8)
    if (keyCode >= '1' && keyCode <= '8')
    {
        const int padIndex = keyCode - '1';
        if (soundboardPanel != nullptr)
        {
            soundboardPanel->triggerPad(padIndex);
        }
        return true;
    }
    else if (keyCode >= juce::KeyPress::numberPad1 && keyCode <= juce::KeyPress::numberPad8)
    {
        const int padIndex = keyCode - juce::KeyPress::numberPad1;
        if (soundboardPanel != nullptr)
        {
            soundboardPanel->triggerPad(padIndex);
        }
        return true;
    }
    else if (keyCode == juce::KeyPress::escapeKey)
    {
        if (soundboardPanel != nullptr)
        {
            soundboardPanel->stopAll();
        }
        return true;
    }
    // Hotkey Tab / F4 to cycle right-hand panel tabs (Master <-> Vocal DSP <-> Soundboard)
    else if (keyCode == juce::KeyPress::tabKey || keyCode == juce::KeyPress::F4Key)
    {
        int nextTab = (static_cast<int>(currentRightTab) + 1) % 3;
        setRightTab(static_cast<RightTab>(nextTab));
        return true;
    }

    return false;
}

void MainComponent::toggleCompactMode()
{
    isCompactMode = !isCompactMode;

    if (auto* dw = findParentComponentOfClass<juce::DocumentWindow>())
    {
        const int compactContentHeight = 64;
        int frameH = (dw->getPeer() != nullptr) ? dw->getPeer()->getFrameSize().getTopAndBottom() : 45;
        if (frameH < 30)
            frameH = 45;

        const int totalCompactHeight = compactContentHeight + frameH;

        if (isCompactMode)
        {
            // Close any active overlays before entering compact mode
            if (isSettingsOverlayVisible)
            {
                isSettingsOverlayVisible = false;
                if (settingsOverlay != nullptr) settingsOverlay->setVisible(false);
            }
            if (isDonateOverlayVisible)
            {
                isDonateOverlayVisible = false;
                if (donateOverlay != nullptr) donateOverlay->setVisible(false);
            }
            if (isSongbookOverlayVisible)
            {
                isSongbookOverlayVisible = false;
                if (songbookOverlay != nullptr) songbookOverlay->setVisible(false);
            }
#if HOSI_PRO_EDITION
            if (isYouTubeOverlayVisible)
            {
                isYouTubeOverlayVisible = false;
                if (youtubeOverlay != nullptr) youtubeOverlay->setVisible(false);
            }
#endif
            wasInCompactModeBeforeOverlay = false;

            // Entering Streamer Mini-Bar Mode
            previousFullWidth = std::max(1000, dw->getWidth());
            previousFullHeight = std::max(600, dw->getHeight());

            compactModeButton.setMiniMode(true);
            compactModeButton.setTooltip(juce::String::fromUTF8(u8"Mở rộng lại Full Studio Rack"));

            if (keyDetectorBar != nullptr)
                keyDetectorBar->setVisible(false);
            masterTabButton.setVisible(false);
            vocalDspTabButton.setVisible(false);
            soundboardTabButton.setVisible(false);
            if (verticalRack != nullptr)
                verticalRack->setVisible(false);
            if (masterStrip != nullptr)
                masterStrip->setVisible(false);
            if (builtInDspPanel != nullptr)
                builtInDspPanel->setVisible(false);
            if (soundboardPanel != nullptr)
                soundboardPanel->setVisible(false);

            dw->setResizeLimits(700, totalCompactHeight, 1920, totalCompactHeight);
            dw->setSize(dw->getWidth(), totalCompactHeight);
        }
        else
        {
            // Expanding back to Full Studio Rack Mode
            compactModeButton.setMiniMode(false);
            compactModeButton.setTooltip(juce::String::fromUTF8(u8"Thu gọn thành Mini Bar nổi (Streamer Overlay)"));

            if (keyDetectorBar != nullptr)
                keyDetectorBar->setVisible(true);
            masterTabButton.setVisible(true);
            vocalDspTabButton.setVisible(true);
            soundboardTabButton.setVisible(true);
            if (verticalRack != nullptr)
                verticalRack->setVisible(true);
            
            if (masterStrip != nullptr)
                masterStrip->setVisible(currentRightTab == RightTab::Master);
            if (builtInDspPanel != nullptr)
                builtInDspPanel->setVisible(currentRightTab == RightTab::VocalDsp);
            if (soundboardPanel != nullptr)
                soundboardPanel->setVisible(currentRightTab == RightTab::Soundboard);

            dw->setResizeLimits(700, 500, 1920, 1200);
            dw->setSize(previousFullWidth, previousFullHeight);
        }
    }

    resized();
    repaint();
}

void MainComponent::applyUiScale(float scaleFactor, int targetW, int targetH)
{
    if (isCompactMode)
    {
        toggleCompactMode(); // expand first if in mini mode
    }

    scaleFactor = std::clamp(scaleFactor, 0.70f, 1.60f);
    presetManager.setUiScale(scaleFactor);

    juce::Desktop::getInstance().setGlobalScaleFactor(scaleFactor);

    if (targetW > 0 && targetH > 0)
    {
        previousFullWidth = targetW;
        previousFullHeight = targetH;
        presetManager.setWindowSize(targetW, targetH);
    }

    if (auto* dw = findParentComponentOfClass<juce::DocumentWindow>())
    {
        dw->setSize(previousFullWidth, previousFullHeight);
        dw->centreWithSize(previousFullWidth, previousFullHeight);
    }
    else
    {
        setSize(previousFullWidth, previousFullHeight);
    }

    if (settingsOverlay != nullptr && settingsOverlay->isVisible())
    {
        settingsOverlay->updateScaleButtonsUI(scaleFactor);
    }

    resized();
    repaint();
}

void MainComponent::toggleAlwaysOnTop()
{
    isAlwaysOnTop = !isAlwaysOnTop;

    if (auto* dw = findParentComponentOfClass<juce::DocumentWindow>())
    {
        dw->setAlwaysOnTop(isAlwaysOnTop);
    }

    if (isAlwaysOnTop)
    {
        alwaysOnTopButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xfff59e0b)); // Gold
        alwaysOnTopButton.setColour(juce::TextButton::textColourOffId, juce::Colours::black);
        alwaysOnTopButton.setTooltip(juce::String::fromUTF8(u8"Ghim cửa sổ luôn nổi (Đang BẬT) - Click để bỏ ghim"));
    }
    else
    {
        alwaysOnTopButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff1e293b));
        alwaysOnTopButton.setColour(juce::TextButton::textColourOffId, juce::Colour(0xff94a3b8));
        alwaysOnTopButton.setTooltip(juce::String::fromUTF8(u8"Ghim cửa sổ luôn nổi trên cùng (Always On Top)"));
    }
}

void MainComponent::handleSavePreset()
{
    auto presetsDir = presetManager.getPresetsDirectory();

    auto fileChooser = std::make_shared<juce::FileChooser>(
        "Save Vocal Chain Preset",
        presetsDir.getChildFile("My Vocal Preset.dawpreset"),
        "*.dawpreset;*.xml"
    );

    fileChooser->launchAsync(juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles | juce::FileBrowserComponent::warnAboutOverwriting,
        [this, fileChooser](const juce::FileChooser& fc)
        {
            auto file = fc.getResult();
            if (file != juce::File())
            {
                if (file.getFileExtension().isEmpty())
                    file = file.withFileExtension("dawpreset");

                if (presetManager.savePresetToFile(file))
                {
                    juce::AlertWindow::showMessageBoxAsync(
                        juce::AlertWindow::InfoIcon,
                        "Preset Saved",
                        "Preset successfully saved:\n" + file.getFileName(),
                        "OK"
                    );
                }
                else
                {
                    juce::AlertWindow::showMessageBoxAsync(
                        juce::AlertWindow::WarningIcon,
                        "Error Saving Preset",
                        "Failed to write preset file.",
                        "OK"
                    );
                }
            }
        });
}

void MainComponent::handleLoadPreset()
{
    auto presetsDir = presetManager.getPresetsDirectory();

    auto fileChooser = std::make_shared<juce::FileChooser>(
        "Load Vocal Chain Preset",
        presetsDir,
        "*.dawpreset;*.xml"
    );

    fileChooser->launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
        [this, fileChooser](const juce::FileChooser& fc)
        {
            auto file = fc.getResult();
            if (file.existsAsFile())
            {
                if (presetManager.loadPresetFromFile(file))
                {
                    if (verticalRack != nullptr)
                        verticalRack->rebuildSlotUI();

                    if (masterStrip != nullptr)
                        masterStrip->updateAllUI();

                    updateMuteButtonUI();
                    updateInputSourceButtonUI();
                    updatePresetButtonsUI();

                    juce::AlertWindow::showMessageBoxAsync(
                        juce::AlertWindow::InfoIcon,
                        "Preset Loaded",
                        "Successfully loaded preset:\n" + file.getFileNameWithoutExtension(),
                        "OK"
                    );
                }
                else
                {
                    juce::AlertWindow::showMessageBoxAsync(
                        juce::AlertWindow::WarningIcon,
                        "Error Loading Preset",
                        "Failed to parse or load preset file.",
                        "OK"
                    );
                }
            }
        });
}

void MainComponent::timerCallback()
{
    const auto mode = audioEngine.getCurrentApiMode();
    const double sr = audioEngine.getSampleRate();
    const int bs = audioEngine.getBufferSize();
    const float latencyMs = (sr > 0.0) ? static_cast<float>((static_cast<double>(bs) / sr) * 1000.0) : 0.0f;
    const float cpu = audioEngine.getCpuUsage();

    // Smooth CPU with exponential moving average to avoid jitter
    smoothedCpuUsage = (smoothedCpuUsage * 0.85f) + (cpu * 0.15f);

    if (mode == LiveStreamIPC::AudioApiMode::AsioExclusive)
    {
        modeBadgeLabel.setText("ASIO PRO / DIRECT MODE", juce::dontSendNotification);
        modeBadgeLabel.setColour(juce::Label::backgroundColourId, juce::Colour(0x3300e676));
        modeBadgeLabel.setColour(juce::Label::textColourId, juce::Colour(0xff00e676));
    }
    else
    {
        modeBadgeLabel.setText("WASAPI SHARED (OBS IPC)", juce::dontSendNotification);
        modeBadgeLabel.setColour(juce::Label::backgroundColourId, juce::Colour(0x330284c7));
        modeBadgeLabel.setColour(juce::Label::textColourId, juce::Colour(0xff38bdf8));
    }

    // Refresh stats string without flicker
    ++timerTicks;
    if (timerTicks % 6 == 0) // ~5 updates per second
    {
        const juce::String newStats = juce::String::formatted(
            "%.1f kHz | %d spls (%.1f ms) | CPU: %.0f%%",
            sr / 1000.0, bs, latencyMs, std::round(smoothedCpuUsage)
        );

        if (statsLabel.getText() != newStats)
        {
            statsLabel.setText(newStats, juce::dontSendNotification);
        }
    }
}
