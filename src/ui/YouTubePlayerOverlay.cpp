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

    // Pitch Controls
    pitchLabel.setFont(juce::FontOptions(13.0f, juce::Font::bold));
    pitchLabel.setColour(juce::Label::textColourId, juce::Colour(0xff38bdf8));
    addAndMakeVisible(pitchLabel);

    auto setupPitchBtn = [this](juce::TextButton& btn, int shift) {
        btn.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff1e293b));
        btn.setColour(juce::TextButton::textColourOffId, juce::Colour(0xff38bdf8));
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

    // Header Row 2 (Search Editor + Search Button + Pitch Controls)
    auto searchRow = bounds.removeFromTop(38).reduced(0, 2);
    
    // Right side: Pitch buttons (-3, -2, -1, 0, +1, +2, +3)
    pitchUp3Btn.setBounds(searchRow.removeFromRight(28).reduced(1));
    pitchUp2Btn.setBounds(searchRow.removeFromRight(28).reduced(1));
    pitchUp1Btn.setBounds(searchRow.removeFromRight(28).reduced(1));
    pitchResetBtn.setBounds(searchRow.removeFromRight(28).reduced(1));
    pitchDown1Btn.setBounds(searchRow.removeFromRight(28).reduced(1));
    pitchDown2Btn.setBounds(searchRow.removeFromRight(28).reduced(1));
    pitchDown3Btn.setBounds(searchRow.removeFromRight(28).reduced(1));
    pitchLabel.setBounds(searchRow.removeFromRight(44));

    searchRow.removeFromRight(6);
    searchButton.setBounds(searchRow.removeFromRight(90));
    searchRow.removeFromRight(6);
    searchEditor.setBounds(searchRow);

    bounds.removeFromTop(6);

    // WebBrowser fills the rest
    if (webBrowser != nullptr)
    {
        webBrowser->setBounds(bounds);
    }
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

    // Calculate transposed root note for Auto-Tune
    int transposedRoot = (currentBaseRootNote + semitones) % 12;
    if (transposedRoot < 0) transposedRoot += 12;

    const juce::String transposedKeyName = KeyDetector::formatKeyName(transposedRoot, currentBaseScale);
    
    // Update Pitch Label on Toolbar in Realtime
    juce::String shiftText = (semitones > 0 ? "+" : "") + juce::String(semitones);
    if (semitones == 0)
        pitchLabel.setText("TONE: " + transposedKeyName, juce::dontSendNotification);
    else
        pitchLabel.setText("TONE: " + transposedKeyName + " (" + shiftText + ")", juce::dontSendNotification);

    // Automatically sync the transposed Tone into Auto-Tune without blocking popup
    if (onApplyTone)
    {
        onApplyTone(transposedRoot, currentBaseScale, 
                    (activeSongName.isNotEmpty() ? activeSongName : juce::String::fromUTF8(u8"YouTube Beat")) + " [" + transposedKeyName + " (" + shiftText + ")]", 
                    false);
    }

    // Calculate HTML5 playback rate corresponding to semitone shift
    // semitone formula: rate = 2^(semitones / 12)
    float rate = std::pow(2.0f, static_cast<float>(semitones) / 12.0f);
    
    // Inject persistent JavaScript pitch shifting hook
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
    // 1. First, try to read the active YouTube video title via JavaScript
    juce::String jsGetTitle = 
        "(function() {"
        "  var t = '';"
        "  var el = document.querySelector('h1.ytd-watch-metadata yt-formatted-string, #title h1 yt-formatted-string, ytd-watch-metadata #title, h1.title, #video-title');"
        "  if (el && el.innerText) { t = el.innerText.trim(); }"
        "  else { t = document.title || ''; }"
        "  return t;"
        "})();";

    auto handleTitleAnalysis = [this](const juce::String& videoTitle) {
        juce::String title = videoTitle.trim();
        if (title.isEmpty())
            title = searchEditor.getText().trim();

        bool toneFound = false;
        juce::String detectedTone;
        int rootNote = 0;
        bool isMinor = false;
        juce::String songMatchedName = title;

        // --- Regex / Pattern check for explicit key in title ---
        // (Am), [Am], (Tone Nam: Dm), (Tone Nữ: Gm), Tone Dm, Tone Am, Am, Em, etc.
        const char* allNotes[] = { "C#m", "D#m", "F#m", "G#m", "A#m", "Db", "Eb", "Gb", "Ab", "Bb", 
                                   "C#", "D#", "F#", "G#", "A#", "Am", "Bm", "Cm", "Dm", "Em", "Fm", "Gm",
                                   "C", "D", "E", "F", "G", "A", "B" };

        for (const char* note : allNotes)
        {
            juce::String noteStr = note;
            // Check enclosed format: (Am), [Am], || Am ||, - Am, Tone Am, Tone: Am
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

        // --- Check Database search in SongbookManager ---
        if (!toneFound && songbookManager != nullptr)
        {
            // Clean title to extract song name
            juce::String query = title;
            query = query.replace("karaoke", "", true)
                         .replace("beat", "", true)
                         .replace("chuan", "", true)
                         .replace(juce::String::fromUTF8(u8"chuẩn"), "", true)
                         .replace("tone nam", "", true)
                         .replace(juce::String::fromUTF8(u8"tone nữ"), "", true)
                         .replace("giong nam", "", true)
                         .replace(juce::String::fromUTF8(u8"giọng nam"), "", true)
                         .replace("giong nu", "", true)
                         .replace(juce::String::fromUTF8(u8"giọng nữ"), "", true)
                         .replace("official", "", true)
                         .replace("remix", "", true)
                         .replace("mv", "", true)
                         .replace("4k", "", true)
                         .replace("hd", "", true)
                         .replace("mashup", "", true)
                         .replace("nhac song", "", true)
                         .replace(juce::String::fromUTF8(u8"nhạc sống"), "", true)
                         .replace("acoustic", "", true)
                         .replace("phoi chuan", "", true)
                         .replace(juce::String::fromUTF8(u8"phối chuẩn"), "", true)
                         .trimCharactersAtStart("- |–[]()")
                         .trimCharactersAtEnd("- |–[]()")
                         .trim();

            auto results = songbookManager->searchSongs(query);
            if (!results.empty())
            {
                const auto& song = results.front();
                songMatchedName = song.title;
                if (title.containsIgnoreCase("nu") || title.containsIgnoreCase("nữ") || title.containsIgnoreCase("female"))
                {
                    detectedTone = song.keyFemale;
                }
                else if (title.containsIgnoreCase("nam") || title.containsIgnoreCase("male"))
                {
                    detectedTone = song.keyMale;
                }
                else
                {
                    detectedTone = song.getEffectiveTone();
                }
                toneFound = true;
            }
        }

        if (toneFound && detectedTone.isNotEmpty())
        {
            SongbookManager::parseKeyAndScale(detectedTone, rootNote, isMinor);
            auto scaleType = isMinor ? KeyDetector::ScaleType::Minor : KeyDetector::ScaleType::Major;

            currentBaseRootNote = rootNote;
            currentBaseScale = scaleType;
            hasActiveBaseTone = true;
            activeSongName = songMatchedName;
            currentPitchShift = 0;
            pitchLabel.setText("TONE: " + KeyDetector::formatKeyName(rootNote, scaleType), juce::dontSendNotification);

            if (onApplyTone)
            {
                onApplyTone(rootNote, scaleType, songMatchedName + " [" + detectedTone + "]", true);
            }

            juce::String alertMsg = juce::String::fromUTF8(u8"✓ Tự động nhận diện Tone bài hát từ YouTube:\n\n")
                                  + juce::String::fromUTF8(u8"• Bài hát / Video: ") + songMatchedName + "\n"
                                  + juce::String::fromUTF8(u8"• Tone phát hiện: [") + detectedTone + (isMinor ? " (Thứ)]\n" : " (Trưởng)]\n")
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



