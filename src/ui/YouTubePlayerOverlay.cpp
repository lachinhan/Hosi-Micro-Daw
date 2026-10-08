#include "YouTubePlayerOverlay.h"

YouTubePlayerOverlay::YouTubePlayerOverlay(SongbookManager* songbookMgr)
    : songbookManager(songbookMgr)
{
    // Title
    titleLabel.setText(juce::String::fromUTF8(u8"📺 MINI YOUTUBE KARAOKE PLAYER (SIÊU NHẸ)"), juce::dontSendNotification);
    titleLabel.setFont(juce::FontOptions(16.0f, juce::Font::bold));
    titleLabel.setColour(juce::Label::textColourId, juce::Colour(0xfff43f5e)); // Rose red
    addAndMakeVisible(titleLabel);

    // Close Button
    closeButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0x33ffffff));
    closeButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    closeButton.onClick = [this]() {
        if (onCloseClicked)
            onCloseClicked();
    };
    addAndMakeVisible(closeButton);

    // Tone & Auto-Tune Direct Controls
    detectKeyButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff059669)); // Emerald Green
    detectKeyButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    detectKeyButton.setTooltip(juce::String::fromUTF8(u8"Dò Tone tự động từ Beat / Video đang phát và truyền thẳng vào Auto-Tune"));
    detectKeyButton.onClick = [this]() {
        detectKeyFromYouTubeTitleOrAudio();
    };
    addAndMakeVisible(detectKeyButton);

    manualToneButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff0284c7)); // Sky Blue
    manualToneButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    manualToneButton.setTooltip(juce::String::fromUTF8(u8"Chọn nhanh Tone bài hát (Major/Minor) và gửi ngay vào Auto-Tune"));
    manualToneButton.onClick = [this]() {
        showManualToneMenu();
    };
    addAndMakeVisible(manualToneButton);

    songbookQuickButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff7c3aed)); // Violet
    songbookQuickButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    songbookQuickButton.setTooltip(juce::String::fromUTF8(u8"Mở Sổ Tone 1.033+ bài hát để tra cứu tone chuẩn Nam / Nữ"));
    songbookQuickButton.onClick = [this]() {
        if (onOpenSongbook)
            onOpenSongbook();
    };
    addAndMakeVisible(songbookQuickButton);

    // Search Box
    searchEditor.setTextToShowWhenEmpty(juce::String::fromUTF8(u8"🔍 Gõ tên bài hát để tìm beat Karaoke trên YouTube (ví dụ: hoa no khong mau karaoke)..."), juce::Colour(0xff94a3b8));
    searchEditor.setColour(juce::TextEditor::backgroundColourId, juce::Colour(0xff1e293b));
    searchEditor.setColour(juce::TextEditor::textColourId, juce::Colours::white);
    searchEditor.setColour(juce::TextEditor::outlineColourId, juce::Colour(0xff475569));
    searchEditor.setColour(juce::TextEditor::focusedOutlineColourId, juce::Colour(0xfff43f5e));
    searchEditor.addListener(this);
    addAndMakeVisible(searchEditor);

    // Search Button
    searchButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xffe11d48)); // Crimson / YouTube Red
    searchButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    searchButton.onClick = [this]() {
        searchAndPlay(searchEditor.getText().trim());
    };
    addAndMakeVisible(searchButton);

    // Clean Mode / Ad-Skip Toggle
    cleanModeButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff065f46)); // Emerald
    cleanModeButton.setColour(juce::TextButton::textColourOffId, juce::Colour(0xffa7f3d0));
    cleanModeButton.setTooltip(juce::String::fromUTF8(u8"Tự động kích hoạt chế độ chặn quảng cáo & tối ưu giao diện hát"));
    cleanModeButton.onClick = [this]() {
        injectAdSkipScript();
    };
    addAndMakeVisible(cleanModeButton);

    // Refresh Button
    refreshButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff334155));
    refreshButton.setColour(juce::TextButton::textColourOffId, juce::Colour(0xffcbd5e1));
    refreshButton.onClick = [this]() {
        loadUrl(currentUrl);
    };
    addAndMakeVisible(refreshButton);

    // Pitch Label for Beat
    pitchLabel.setFont(juce::FontOptions(11.5f, juce::Font::bold));
    pitchLabel.setColour(juce::Label::textColourId, juce::Colour(0xff38bdf8));
    addAndMakeVisible(pitchLabel);

    // Song Ca / Duet Buttons (Chuyển Tone Auto-Tune cho Nam / Nữ)
    maleToneButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff1e293b));
    maleToneButton.setColour(juce::TextButton::textColourOffId, juce::Colour(0xff60a5fa));
    maleToneButton.setTooltip(juce::String::fromUTF8(u8"Chuyển Auto-Tune sang Tone Nam (Phím tắt: M hoặc 1)"));
    maleToneButton.onClick = [this]() { selectMaleTone(true); };
    addAndMakeVisible(maleToneButton);

    femaleToneButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff1e293b));
    femaleToneButton.setColour(juce::TextButton::textColourOffId, juce::Colour(0xfff472b6));
    femaleToneButton.setTooltip(juce::String::fromUTF8(u8"Chuyển Auto-Tune sang Tone Nữ (Phím tắt: F hoặc 2)"));
    femaleToneButton.onClick = [this]() { selectFemaleTone(true); };
    addAndMakeVisible(femaleToneButton);

    // Vocal Auto-Tune Modulation Controls (Lên/Hạ Tone Auto-Tune khi nhạc chuyển tone giữa/cuối bài)
    vocalModLabel.setFont(juce::FontOptions(11.0f, juce::Font::bold));
    vocalModLabel.setColour(juce::Label::textColourId, juce::Colour(0xfff59e0b)); // Amber Gold
    addAndMakeVisible(vocalModLabel);

    auto setupVocalModBtn = [this](juce::TextButton& btn, int shift) {
        btn.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff1e293b));
        btn.setColour(juce::TextButton::textColourOffId, juce::Colour(0xfffbbf24));
        juce::String shiftStr = (shift > 0 ? "+" : "") + juce::String(shift);
        if (shift == 0)
            btn.setTooltip(juce::String::fromUTF8(u8"Trả Auto-Tune về Tone gốc của bài hát (Phím tắt: 0)"));
        else
            btn.setTooltip(juce::String::fromUTF8(u8"Lên Tone Auto-Tune ") + shiftStr + juce::String::fromUTF8(u8" nửa cung khi bài hát chuyển đoạn/lên tone (Phím tắt: [ hoặc ])"));
        btn.onClick = [this, shift]() {
            executeVocalToneModulation(shift);
        };
        addAndMakeVisible(btn);
    };

    setupVocalModBtn(vocalModDown1Btn, -1);
    setupVocalModBtn(vocalModResetBtn, 0);
    setupVocalModBtn(vocalModUp1Btn, 1);
    setupVocalModBtn(vocalModUp2Btn, 2);

    updateDuetButtonsUI();
    updateVocalModButtonsUI();
    setWantsKeyboardFocus(true);

    // Beat Pitch Shifter Buttons (Nâng/hạ Tone Beat YouTube)
    auto setupPitchBtn = [this](juce::TextButton& btn, int shift) {
        btn.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff1e293b));
        btn.setColour(juce::TextButton::textColourOffId, juce::Colour(0xff38bdf8));
        juce::String shiftStr = (shift > 0 ? "+" : "") + juce::String(shift);
        if (shift == 0)
            btn.setTooltip(juce::String::fromUTF8(u8"Trả Beat YouTube & Auto-Tune về Tone gốc ban đầu (Phím tắt: 0)"));
        else
            btn.setTooltip(juce::String::fromUTF8(u8"Tăng/Giảm cao độ Beat YouTube ") + shiftStr + juce::String::fromUTF8(u8" nửa cung & đồng bộ Auto-Tune (Phím tắt: + / -)"));
        btn.onClick = [this, shift]() {
            executePitchShift(shift);
        };
        addAndMakeVisible(btn);
    };

    setupPitchBtn(pitchDown3Btn, -3);
    setupPitchBtn(pitchDown2Btn, -2);
    setupPitchBtn(pitchDown1Btn, -1);
    setupPitchBtn(pitchResetBtn, 0);
    setupPitchBtn(pitchUp1Btn, 1);
    setupPitchBtn(pitchUp2Btn, 2);
    setupPitchBtn(pitchUp3Btn, 3);

    // Create Native WebBrowserComponent with WebView2 on Windows
    try
    {
#if JUCE_WINDOWS
        auto options = juce::WebBrowserComponent::Options{}.withBackend(juce::WebBrowserComponent::Options::Backend::webview2);
        webBrowser = std::make_unique<juce::WebBrowserComponent>(options);
#else
        webBrowser = std::make_unique<juce::WebBrowserComponent>();
#endif
        addAndMakeVisible(webBrowser.get());
        
        // Initial page: YouTube Karaoke Portal
        loadUrl(currentUrl);
    }
    catch (...)
    {
    }
}

void YouTubePlayerOverlay::paint(juce::Graphics& g)
{
    // Semi-transparent backdrop with sleek border
    g.fillAll(juce::Colour(0xeb0f172a)); // Slate 900 Glassmorphism

    auto bounds = getLocalBounds().toFloat();
    g.setColour(juce::Colour(0xff334155));
    g.drawRect(bounds, 1.5f);

    // Top Header bar background
    g.setColour(juce::Colour(0xff181e2e));
    g.fillRect(0, 0, getWidth(), 84);
    g.setColour(juce::Colour(0xff334155));
    g.drawHorizontalLine(84, 0.0f, static_cast<float>(getWidth()));
}

void YouTubePlayerOverlay::resized()
{
    auto bounds = getLocalBounds().reduced(8);

    // Header Row 1 (Title + Quick Action Buttons + Close)
    auto topRow = bounds.removeFromTop(36);
    closeButton.setBounds(topRow.removeFromRight(36).reduced(2));
    refreshButton.setBounds(topRow.removeFromRight(76).reduced(2));
    cleanModeButton.setBounds(topRow.removeFromRight(126).reduced(2));
    
    // Auto-Tune & Tone Buttons on Right side of Row 1
    songbookQuickButton.setBounds(topRow.removeFromRight(95).reduced(2));
    manualToneButton.setBounds(topRow.removeFromRight(135).reduced(2));
    detectKeyButton.setBounds(topRow.removeFromRight(155).reduced(2));

    titleLabel.setBounds(topRow);

    // Header Row 2 (Search + Duet + Vocal Mod + Beat Pitch)
    auto searchRow = bounds.removeFromTop(38).reduced(0, 2);
    
    // Right Section 1: Beat Pitch Shift Buttons (-3, -2, -1, 0, +1, +2, +3)
    pitchUp3Btn.setBounds(searchRow.removeFromRight(26).reduced(1));
    pitchUp2Btn.setBounds(searchRow.removeFromRight(26).reduced(1));
    pitchUp1Btn.setBounds(searchRow.removeFromRight(26).reduced(1));
    pitchResetBtn.setBounds(searchRow.removeFromRight(26).reduced(1));
    pitchDown1Btn.setBounds(searchRow.removeFromRight(26).reduced(1));
    pitchDown2Btn.setBounds(searchRow.removeFromRight(26).reduced(1));
    pitchDown3Btn.setBounds(searchRow.removeFromRight(26).reduced(1));
    pitchLabel.setBounds(searchRow.removeFromRight(42));

    searchRow.removeFromRight(4);

    // Right Section 2: Vocal Auto-Tune Modulation Buttons (Lên Tone theo bài: -1, 0, +1, +2)
    vocalModUp2Btn.setBounds(searchRow.removeFromRight(26).reduced(1));
    vocalModUp1Btn.setBounds(searchRow.removeFromRight(26).reduced(1));
    vocalModResetBtn.setBounds(searchRow.removeFromRight(26).reduced(1));
    vocalModDown1Btn.setBounds(searchRow.removeFromRight(26).reduced(1));
    vocalModLabel.setBounds(searchRow.removeFromRight(56));

    searchRow.removeFromRight(4);

    // Center Section: Duet Male / Female buttons
    femaleToneButton.setBounds(searchRow.removeFromRight(60).reduced(1));
    maleToneButton.setBounds(searchRow.removeFromRight(60).reduced(1));

    searchRow.removeFromRight(6);
    searchButton.setBounds(searchRow.removeFromRight(80));
    searchRow.removeFromRight(6);
    searchEditor.setBounds(searchRow);

    bounds.removeFromTop(6);

    // WebBrowser fills the rest
    if (webBrowser != nullptr)
    {
        webBrowser->setBounds(bounds);
    }
}

bool YouTubePlayerOverlay::keyPressed(const juce::KeyPress& key)
{
    if (searchEditor.hasKeyboardFocus(true))
        return false;

    auto keyCode = key.getKeyCode();
    auto textChar = key.getTextCharacter();

    // Hotkey [ / ] to modulate Vocal Auto-Tune Tone when song modulates (+1, +2, -1, 0)
    if (textChar == ']' || keyCode == juce::KeyPress::pageUpKey)
    {
        executeVocalToneModulation(std::clamp(currentVocalModulation + 1, -2, 3));
        return true;
    }
    else if (textChar == '[' || keyCode == juce::KeyPress::pageDownKey)
    {
        executeVocalToneModulation(std::clamp(currentVocalModulation - 1, -2, 3));
        return true;
    }

    // Hotkey +/- or Up/Down arrows to transpose YouTube Beat pitch (+ Auto-Tune)
    if (textChar == '+' || textChar == '=' || keyCode == juce::KeyPress::upKey)
    {
        executePitchShift(std::clamp(currentPitchShift + 1, -6, 6));
        return true;
    }
    else if (textChar == '-' || textChar == '_' || keyCode == juce::KeyPress::downKey)
    {
        executePitchShift(std::clamp(currentPitchShift - 1, -6, 6));
        return true;
    }
    else if (textChar == '0' || keyCode == juce::KeyPress::numberPad0)
    {
        if (currentVocalModulation != 0)
            executeVocalToneModulation(0);
        else
            executePitchShift(0);
        return true;
    }
    // Hotkey M / 1 to switch Male Tone
    else if (textChar == 'm' || textChar == 'M' || textChar == '1' || keyCode == juce::KeyPress::numberPad1)
    {
        selectMaleTone(true);
        return true;
    }
    // Hotkey F / 2 to switch Female Tone
    else if (textChar == 'f' || textChar == 'F' || textChar == '2' || keyCode == juce::KeyPress::numberPad2)
    {
        selectFemaleTone(true);
        return true;
    }
    // Hotkey D / Tab to toggle duet tone
    else if (textChar == 'd' || textChar == 'D' || keyCode == juce::KeyPress::tabKey)
    {
        toggleDuetTone();
        return true;
    }

    return false;
}

void YouTubePlayerOverlay::updateDuetButtonsUI()
{
    maleToneButton.setButtonText(juce::String::fromUTF8(u8"♂ ") + maleTone);
    femaleToneButton.setButtonText(juce::String::fromUTF8(u8"♀ ") + femaleTone);

    if (activeGender == ActiveDuetGender::Male)
    {
        maleToneButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff2563eb)); // Bright Royal Blue
        maleToneButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
        femaleToneButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff1e293b)); // Slate
        femaleToneButton.setColour(juce::TextButton::textColourOffId, juce::Colour(0xfff472b6)); // Pink text
    }
    else if (activeGender == ActiveDuetGender::Female)
    {
        femaleToneButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xffdb2777)); // Vibrant Magenta / Rose
        femaleToneButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
        maleToneButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff1e293b)); // Slate
        maleToneButton.setColour(juce::TextButton::textColourOffId, juce::Colour(0xff60a5fa)); // Blue text
    }
    else
    {
        maleToneButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff1e293b));
        maleToneButton.setColour(juce::TextButton::textColourOffId, juce::Colour(0xff60a5fa));
        femaleToneButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff1e293b));
        femaleToneButton.setColour(juce::TextButton::textColourOffId, juce::Colour(0xfff472b6));
    }
    maleToneButton.repaint();
    femaleToneButton.repaint();
}

void YouTubePlayerOverlay::updateVocalModButtonsUI()
{
    auto styleModBtn = [this](juce::TextButton& btn, int val) {
        bool active = (currentVocalModulation == val);
        btn.setColour(juce::TextButton::buttonColourId, active ? juce::Colour(0xffd97706) : juce::Colour(0xff1e293b)); // Amber
        btn.setColour(juce::TextButton::textColourOffId, active ? juce::Colours::white : juce::Colour(0xfffbbf24));
    };

    styleModBtn(vocalModDown1Btn, -1);
    styleModBtn(vocalModResetBtn, 0);
    styleModBtn(vocalModUp1Btn, 1);
    styleModBtn(vocalModUp2Btn, 2);
}

void YouTubePlayerOverlay::syncToneToAutoTuneAndUI()
{
    int totalShift = currentPitchShift + currentVocalModulation;
    int effectiveRoot = (currentBaseRootNote + totalShift) % 12;
    if (effectiveRoot < 0) effectiveRoot += 12;

    const juce::String effectiveKeyName = KeyDetector::formatKeyName(effectiveRoot, currentBaseScale);

    juce::String genderSuffix = (activeGender == ActiveDuetGender::Female) 
        ? juce::String::fromUTF8(u8" (♀ Nữ)") 
        : juce::String::fromUTF8(u8" (♂ Nam)");

    juce::String statusText = "TONE: " + effectiveKeyName + genderSuffix;
    if (currentVocalModulation != 0)
    {
        juce::String sign = (currentVocalModulation > 0 ? "+" : "");
        statusText += " [Mod " + sign + juce::String(currentVocalModulation) + "]";
    }
    if (currentPitchShift != 0)
    {
        juce::String sign = (currentPitchShift > 0 ? "+" : "") ;
        statusText += " [Beat " + sign + juce::String(currentPitchShift) + "]";
    }

    pitchLabel.setText(statusText, juce::dontSendNotification);

    if (onApplyTone)
    {
        onApplyTone(effectiveRoot, currentBaseScale, 
                    (activeSongName.isNotEmpty() ? activeSongName : juce::String::fromUTF8(u8"YouTube Beat")) + " [" + effectiveKeyName + "]", 
                    false);
    }
}

void YouTubePlayerOverlay::executeVocalToneModulation(int semitones)
{
    currentVocalModulation = semitones;
    updateVocalModButtonsUI();
    syncToneToAutoTuneAndUI();
}

void YouTubePlayerOverlay::selectMaleTone(bool announce)
{
    int root = 0;
    bool isMinor = false;
    SongbookManager::parseKeyAndScale(maleTone, root, isMinor);
    auto scaleType = isMinor ? KeyDetector::ScaleType::Minor : KeyDetector::ScaleType::Major;

    currentBaseRootNote = root;
    currentBaseScale = scaleType;
    hasActiveBaseTone = true;
    currentVocalModulation = 0;
    activeGender = ActiveDuetGender::Male;

    updateDuetButtonsUI();
    updateVocalModButtonsUI();
    syncToneToAutoTuneAndUI();
}

void YouTubePlayerOverlay::selectFemaleTone(bool announce)
{
    int root = 0;
    bool isMinor = false;
    SongbookManager::parseKeyAndScale(femaleTone, root, isMinor);
    auto scaleType = isMinor ? KeyDetector::ScaleType::Minor : KeyDetector::ScaleType::Major;

    currentBaseRootNote = root;
    currentBaseScale = scaleType;
    hasActiveBaseTone = true;
    currentVocalModulation = 0;
    activeGender = ActiveDuetGender::Female;

    updateDuetButtonsUI();
    updateVocalModButtonsUI();
    syncToneToAutoTuneAndUI();
}

void YouTubePlayerOverlay::toggleDuetTone()
{
    if (activeGender == ActiveDuetGender::Male)
        selectFemaleTone(true);
    else
        selectMaleTone(true);
}

void YouTubePlayerOverlay::textEditorReturnKeyPressed(juce::TextEditor& editor)
{
    if (&editor == &searchEditor)
    {
        searchAndPlay(searchEditor.getText().trim());
    }
}

void YouTubePlayerOverlay::searchAndPlay(const juce::String& songName)
{
    if (songName.isEmpty())
        return;

    searchEditor.setText(songName, juce::dontSendNotification);

    if (songbookManager != nullptr)
    {
        juce::String cleanSearch = SongbookManager::removeVietnameseAccents(songName).toLowerCase().trim();
        const auto& allSongs = songbookManager->getAllSongs();
        for (const auto& s : allSongs)
        {
            juce::String cleanS = SongbookManager::removeVietnameseAccents(s.title).toLowerCase().trim();
            if (cleanSearch == cleanS || cleanSearch.contains(cleanS) || cleanS.contains(cleanSearch))
            {
                activeSongName = s.title;
                maleTone = s.keyMale.isNotEmpty() ? s.keyMale : "Am";
                femaleTone = s.keyFemale.isNotEmpty() ? s.keyFemale : "Dm";
                updateDuetButtonsUI();
                if (s.tempo > 0.0 && onApplyTempo)
                    onApplyTempo(s.tempo, s.title);
                break;
            }
        }
    }

    juce::String query = songName;
    if (!query.containsIgnoreCase("karaoke") && !query.containsIgnoreCase("beat"))
    {
        query += " karaoke beat";
    }

    juce::String encodedQuery = juce::URL::addEscapeChars(query, true);
    juce::String url = "https://www.youtube.com/results?search_query=" + encodedQuery;
    loadUrl(url);
}

void YouTubePlayerOverlay::loadUrl(const juce::String& url)
{
    currentUrl = url;
    if (webBrowser != nullptr)
    {
        webBrowser->goToURL(url);
    }
}

void YouTubePlayerOverlay::executePitchShift(int semitones)
{
    currentPitchShift = semitones;
    
    // Update button colors
    auto stylePitchBtn = [this](juce::TextButton& btn, int val) {
        bool active = (currentPitchShift == val);
        btn.setColour(juce::TextButton::buttonColourId, active ? juce::Colour(0xff0284c7) : juce::Colour(0xff1e293b));
        btn.setColour(juce::TextButton::textColourOffId, active ? juce::Colours::white : juce::Colour(0xff38bdf8));
    };

    stylePitchBtn(pitchDown3Btn, -3);
    stylePitchBtn(pitchDown2Btn, -2);
    stylePitchBtn(pitchDown1Btn, -1);
    stylePitchBtn(pitchResetBtn, 0);
    stylePitchBtn(pitchUp1Btn, 1);
    stylePitchBtn(pitchUp2Btn, 2);
    stylePitchBtn(pitchUp3Btn, 3);

    syncToneToAutoTuneAndUI();

    // Calculate HTML5 playback rate corresponding to semitone shift
    // semitone formula: rate = 2^(semitones / 12)
    float rate = std::pow(2.0f, static_cast<float>(semitones) / 12.0f);
    
    // Inject persistent JavaScript pitch shifting hook into YouTube video
    juce::String jsCode = 
        "(function() {"
        "  window.__hosiPitchShift = " + juce::String(semitones) + ";"
        "  var targetRate = " + juce::String(rate, 4) + ";"
        "  function applyTone(v) {"
        "    if (!v) return;"
        "    try {"
        "      v.preservesPitch = false;"
        "      v.mozPreservesPitch = false;"
        "      v.webkitPreservesPitch = false;"
        "      if (Math.abs(v.playbackRate - targetRate) > 0.001) {"
        "        v.playbackRate = targetRate;"
        "      }"
        "    } catch(e) {}"
        "  }"
        "  document.querySelectorAll('video').forEach(applyTone);"
        "  if (!window.__hosiToneHookInstalled) {"
        "    window.__hosiToneHookInstalled = true;"
        "    ['play', 'playing', 'loadedmetadata', 'timeupdate', 'ratechange'].forEach(function(evt) {"
        "      document.addEventListener(evt, function(e) {"
        "        if (e.target && e.target.tagName === 'VIDEO') {"
        "          var s = window.__hosiPitchShift || 0;"
        "          var r = Math.pow(2.0, s / 12.0);"
        "          try {"
        "            e.target.preservesPitch = false;"
        "            e.target.mozPreservesPitch = false;"
        "            e.target.webkitPreservesPitch = false;"
        "            if (Math.abs(e.target.playbackRate - r) > 0.001) {"
        "              e.target.playbackRate = r;"
        "            }"
        "          } catch(err) {}"
        "        }"
        "      }, true);"
        "    });"
        "    setInterval(function() {"
        "      var s = window.__hosiPitchShift || 0;"
        "      var r = Math.pow(2.0, s / 12.0);"
        "      document.querySelectorAll('video').forEach(function(v) {"
        "        try {"
        "          v.preservesPitch = false;"
        "          v.mozPreservesPitch = false;"
        "          v.webkitPreservesPitch = false;"
        "          if (Math.abs(v.playbackRate - r) > 0.001) {"
        "            v.playbackRate = r;"
        "          }"
        "        } catch(err) {}"
        "      });"
        "    }, 600);"
        "  }"
        "})();";
    
    if (webBrowser != nullptr)
    {
        webBrowser->evaluateJavascript(jsCode);
    }
}

void YouTubePlayerOverlay::injectAdSkipScript()
{
    juce::String js = 
        "(function() {"
        "  function runAdSkip() {"
        "    try {"
        "      var skipBtn = document.querySelector('.ytp-ad-skip-button, .ytp-ad-skip-button-modern, .ytp-skip-ad-button, .ytp-ad-overlay-close-button');"
        "      if (skipBtn) { skipBtn.click(); }"
        "      var adOverlays = document.querySelectorAll('.ytp-ad-overlay-container, .ytp-ad-player-overlay, ytd-banner-promo-renderer');"
        "      adOverlays.forEach(function(el) { el.remove(); });"
        "      var vid = document.querySelector('video');"
        "      if (vid && document.querySelector('.ad-showing')) { vid.currentTime = vid.duration || 9999; }"
        "    } catch(e) {}"
        "  }"
        "  runAdSkip();"
        "  if (!window.__hosiAdSkipHook) {"
        "    window.__hosiAdSkipHook = true;"
        "    setInterval(runAdSkip, 500);"
        "  }"
        "})();";

    if (webBrowser != nullptr)
    {
        webBrowser->evaluateJavascript(js);
    }
}

void YouTubePlayerOverlay::showManualToneMenu()
{
    juce::PopupMenu menu;
    menu.addSectionHeader(juce::String::fromUTF8(u8"CHỌN TONE GỬI THẲNG VÀO AUTO-TUNE"));

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

    menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(&manualToneButton), [this](int result) {
        if (result >= 100 && result < 112)
        {
            int root = result - 100;
            currentBaseRootNote = root;
            currentBaseScale = KeyDetector::ScaleType::Major;
            hasActiveBaseTone = true;
            activeSongName = juce::String::fromUTF8(u8"YouTube Beat");
            currentPitchShift = 0;
            pitchLabel.setText("TONE: " + KeyDetector::formatKeyName(root, KeyDetector::ScaleType::Major), juce::dontSendNotification);
            if (onApplyTone)
                onApplyTone(root, KeyDetector::ScaleType::Major, juce::String::fromUTF8(u8"YouTube Player (Thủ công)"), true);
        }
        else if (result >= 200 && result < 212)
        {
            int root = result - 200;
            currentBaseRootNote = root;
            currentBaseScale = KeyDetector::ScaleType::Minor;
            hasActiveBaseTone = true;
            activeSongName = juce::String::fromUTF8(u8"YouTube Beat");
            currentPitchShift = 0;
            pitchLabel.setText("TONE: " + KeyDetector::formatKeyName(root, KeyDetector::ScaleType::Minor), juce::dontSendNotification);
            if (onApplyTone)
                onApplyTone(root, KeyDetector::ScaleType::Minor, juce::String::fromUTF8(u8"YouTube Player (Thủ công)"), true);
        }
    });
}

void YouTubePlayerOverlay::detectKeyFromYouTubeTitleOrAudio()
{
    // 1. Comprehensive JavaScript to extract video title across all YouTube player styles
    juce::String jsGetTitle = 
        "(function() {"
        "  var t = '';"
        "  var selectors = ["
        "    '.ytp-title-link',"
        "    '.ytp-title-text',"
        "    'h1.ytd-watch-metadata yt-formatted-string',"
        "    '#title h1 yt-formatted-string',"
        "    'ytd-watch-metadata #title',"
        "    'h1.title',"
        "    '#video-title',"
        "    '.ytp-chapter-title-content'"
        "  ];"
        "  for (var i = 0; i < selectors.length; ++i) {"
        "    var el = document.querySelector(selectors[i]);"
        "    if (el) {"
        "      var val = el.innerText || el.textContent || '';"
        "      if (val && val.trim().length > 3) {"
        "        t = val.trim();"
        "        break;"
        "      }"
        "    }"
        "  }"
        "  if (!t) {"
        "    var meta = document.querySelector('meta[name=\"title\"], meta[property=\"og:title\"]');"
        "    if (meta) { t = meta.getAttribute('content') || ''; }"
        "  }"
        "  if (!t) {"
        "    t = document.title || '';"
        "  }"
        "  return t;"
        "})();";

    auto handleTitleAnalysis = [this](const juce::String& rawVideoTitle) {
        juce::String title = rawVideoTitle.trim();
        // Remove generic suffix
        if (title.endsWithIgnoreCase("- YouTube"))
            title = title.substring(0, title.length() - 9).trim();
        if (title.endsWithIgnoreCase("- YouTube Music"))
            title = title.substring(0, title.length() - 15).trim();

        if (title.isEmpty())
            title = searchEditor.getText().trim();

        bool toneFound = false;
        juce::String detectedTone;
        int rootNote = 0;
        bool isMinor = false;
        juce::String songMatchedName = title;
        const SongItem* bestMatch = nullptr;

        // --- 1. Regex / Pattern check for explicit key in title ---
        // Examples: (Am), [Am], (Tone Nam: Dm), (Tone Nữ: Gm), Tone Dm, Tone Am, Am, Em, etc.
        const char* allNotes[] = { "C#m", "D#m", "F#m", "G#m", "A#m", "Db", "Eb", "Gb", "Ab", "Bb", 
                                   "C#", "D#", "F#", "G#", "A#", "Am", "Bm", "Cm", "Dm", "Em", "Fm", "Gm",
                                   "C", "D", "E", "F", "G", "A", "B" };

        for (const char* note : allNotes)
        {
            juce::String noteStr = note;
            if (title.containsIgnoreCase("(" + noteStr + ")") ||
                title.containsIgnoreCase("[" + noteStr + "]") ||
                title.containsIgnoreCase(" " + noteStr + " ") ||
                title.containsIgnoreCase("Tone " + noteStr) ||
                title.containsIgnoreCase("Tone: " + noteStr) ||
                title.containsIgnoreCase("Tone:" + noteStr) ||
                title.endsWithIgnoreCase(" " + noteStr) ||
                title.endsWithIgnoreCase("-" + noteStr))
            {
                detectedTone = noteStr;
                toneFound = true;
                break;
            }
        }

        // --- 2. Database scan across 1,033+ Vietnamese songs in SongbookManager ---
        if (!toneFound && songbookManager != nullptr)
        {
            juce::String cleanTitle = SongbookManager::removeVietnameseAccents(title).toLowerCase();
            cleanTitle = cleanTitle.replaceCharacter('-', ' ')
                                   .replaceCharacter('_', ' ')
                                   .replaceCharacter('|', ' ')
                                   .replaceCharacter('(', ' ')
                                   .replaceCharacter(')', ' ')
                                   .replaceCharacter('[', ' ')
                                   .replaceCharacter(']', ' ')
                                   .replaceCharacter(':', ' ')
                                   .replaceCharacter('/', ' ');
            const juce::String paddedTitle = " " + cleanTitle + " ";
            const auto& allSongs = songbookManager->getAllSongs();
            
            int bestMatchLen = 0;

            for (const auto& s : allSongs)
            {
                juce::String cleanSong = SongbookManager::removeVietnameseAccents(s.title).toLowerCase().trim();
                cleanSong = cleanSong.replaceCharacter('-', ' ')
                                     .replaceCharacter('_', ' ')
                                     .replaceCharacter('|', ' ')
                                     .replaceCharacter('(', ' ')
                                     .replaceCharacter(')', ' ')
                                     .replaceCharacter('[', ' ')
                                     .replaceCharacter(']', ' ')
                                     .replaceCharacter(':', ' ')
                                     .replaceCharacter('/', ' ');
                const juce::String paddedSong = " " + cleanSong + " ";

                if (cleanSong.length() >= 3 && paddedTitle.contains(paddedSong))
                {
                    if (cleanSong.length() > bestMatchLen)
                    {
                        bestMatch = &s;
                        bestMatchLen = cleanSong.length();
                    }
                }
            }

            if (bestMatch != nullptr)
            {
                songMatchedName = bestMatch->title;
                maleTone = bestMatch->keyMale.isNotEmpty() ? bestMatch->keyMale : "Am";
                femaleTone = bestMatch->keyFemale.isNotEmpty() ? bestMatch->keyFemale : "Dm";

                if (paddedTitle.contains(" nu ") || paddedTitle.contains(" female ") || paddedTitle.contains(" tone nu ") || paddedTitle.contains(" giong nu "))
                {
                    detectedTone = femaleTone;
                    activeGender = ActiveDuetGender::Female;
                }
                else if (paddedTitle.contains(" nam ") || paddedTitle.contains(" male ") || paddedTitle.contains(" tone nam ") || paddedTitle.contains(" giong nam "))
                {
                    detectedTone = maleTone;
                    activeGender = ActiveDuetGender::Male;
                }
                else
                {
                    detectedTone = bestMatch->getEffectiveTone();
                    activeGender = (detectedTone.equalsIgnoreCase(femaleTone) && !femaleTone.equalsIgnoreCase(maleTone)) ? ActiveDuetGender::Female : ActiveDuetGender::Male;
                }
                toneFound = true;
            }
            else if (detectedTone.isNotEmpty())
            {
                if (paddedTitle.contains(" nu ") || paddedTitle.contains(" female ") || paddedTitle.contains(" tone nu ") || paddedTitle.contains(" giong nu "))
                {
                    femaleTone = detectedTone;
                    maleTone = SongbookManager::transposeKey(detectedTone, -5);
                    activeGender = ActiveDuetGender::Female;
                }
                else
                {
                    maleTone = detectedTone;
                    femaleTone = SongbookManager::transposeKey(detectedTone, 5);
                    activeGender = ActiveDuetGender::Male;
                }
            }
        }

        // --- 3. If Tone detected, apply to Auto-Tune immediately ---
        if (toneFound && detectedTone.isNotEmpty())
        {
            SongbookManager::parseKeyAndScale(detectedTone, rootNote, isMinor);
            auto scaleType = isMinor ? KeyDetector::ScaleType::Minor : KeyDetector::ScaleType::Major;

            currentBaseRootNote = rootNote;
            currentBaseScale = scaleType;
            hasActiveBaseTone = true;
            activeSongName = songMatchedName;
            currentPitchShift = 0;

            juce::String genderSuffix = (activeGender == ActiveDuetGender::Female) 
                ? juce::String::fromUTF8(u8" (♀ Nữ)") 
                : juce::String::fromUTF8(u8" (♂ Nam)");

            pitchLabel.setText("TONE: " + KeyDetector::formatKeyName(rootNote, scaleType) + genderSuffix, juce::dontSendNotification);
            updateDuetButtonsUI();

            if (bestMatch != nullptr && bestMatch->tempo > 0.0 && onApplyTempo)
            {
                onApplyTempo(bestMatch->tempo, songMatchedName);
            }

            if (onApplyTone)
            {
                // showNotificationPopup = false to avoid duplicate popup
                onApplyTone(rootNote, scaleType, songMatchedName + " [" + detectedTone + genderSuffix + "]", false);
            }

            juce::String alertMsg = juce::String::fromUTF8(u8"✓ Đã tự động nhận diện Tone bài hát từ YouTube:\n\n")
                                  + juce::String::fromUTF8(u8"• Bài hát / Video: ") + songMatchedName + "\n"
                                  + juce::String::fromUTF8(u8"• Tone hiện tại: [") + detectedTone + (isMinor ? juce::String::fromUTF8(u8" (Thứ)") : juce::String::fromUTF8(u8" (Trưởng)")) + genderSuffix + "]\n"
                                  + juce::String::fromUTF8(u8"• Song Ca Nam/Nữ: [") + juce::String::fromUTF8(u8"♂ Nam: ") + maleTone + juce::String::fromUTF8(u8" | ♀ Nữ: ") + femaleTone + "]\n\n"
                                  + juce::String::fromUTF8(u8"💡 Phím tắt chuyển Tone nhanh khi hát:\n")
                                  + juce::String::fromUTF8(u8"  • Phím M hoặc 1: Chuyển Auto-Tune sang Tone Nam\n")
                                  + juce::String::fromUTF8(u8"  • Phím F hoặc 2: Chuyển Auto-Tune sang Tone Nữ\n")
                                  + juce::String::fromUTF8(u8"  • Phím + / - : Chuyển Tone Auto-Tune khi bài hát chuyển đoạn/lên tone\n")
                                  + juce::String::fromUTF8(u8"• Đã tự động nạp thành công vào Auto-Tune trong Rack!");

            juce::AlertWindow::showMessageBoxAsync(
                juce::AlertWindow::InfoIcon,
                juce::String::fromUTF8(u8"🎯 DÒ TONE YOUTUBE THÀNH CÔNG"),
                alertMsg,
                "OK"
            );
        }
        else
        {
            // If neither detected from title, trigger audio detection or open manual tone picker
            if (onDetectAndPushToAutoTune)
            {
                onDetectAndPushToAutoTune();
            }
            showManualToneMenu();
        }
    };

    if (webBrowser != nullptr)
    {
        webBrowser->evaluateJavascript(jsGetTitle, [handleTitleAnalysis](juce::WebBrowserComponent::EvaluationResult result) {
            juce::String vTitle;
            if (const auto* r = result.getResult())
            {
                vTitle = r->toString().trim();
            }
            handleTitleAnalysis(vTitle);
        });
    }
    else
    {
        handleTitleAnalysis({});
    }
}



