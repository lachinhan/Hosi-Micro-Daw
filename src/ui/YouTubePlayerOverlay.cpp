#include "YouTubePlayerOverlay.h"

YouTubePlayerOverlay::YouTubePlayerOverlay()
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

    // Header Row 1 (Title + Close + Clean Ad-Skip)
    auto topRow = bounds.removeFromTop(36);
    closeButton.setBounds(topRow.removeFromRight(36).reduced(2));
    refreshButton.setBounds(topRow.removeFromRight(76).reduced(2));
    cleanModeButton.setBounds(topRow.removeFromRight(136).reduced(2));
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

