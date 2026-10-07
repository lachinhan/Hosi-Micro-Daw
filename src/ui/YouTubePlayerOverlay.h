#pragma once

#include <juce_gui_extra/juce_gui_extra.h>

class YouTubePlayerOverlay : public juce::Component, public juce::TextEditor::Listener
{
public:
    YouTubePlayerOverlay();
    ~YouTubePlayerOverlay() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void textEditorReturnKeyPressed(juce::TextEditor& editor) override;

    void searchAndPlay(const juce::String& songName);
    void loadUrl(const juce::String& url);

    std::function<void()> onCloseClicked;

private:
    juce::Label titleLabel;
    juce::TextButton closeButton{ "X" };

    // Search Box
    juce::TextEditor searchEditor;
    juce::TextButton searchButton{ juce::String::fromUTF8(u8"🔍 TÌM BEAT") };

    // Quick Action & Tone Helper Buttons
    juce::TextButton cleanModeButton{ juce::String::fromUTF8(u8"🛡️ CHẶN QC / LIVE") };
    juce::TextButton refreshButton{ juce::String::fromUTF8(u8"🔄 TẢI LẠI") };

    // Pitch Shifter Helper Buttons
    juce::Label pitchLabel{ {}, juce::String::fromUTF8(u8"TONE:") };
    juce::TextButton pitchDown3Btn{ "-3" };
    juce::TextButton pitchDown2Btn{ "-2" };
    juce::TextButton pitchDown1Btn{ "-1" };
    juce::TextButton pitchResetBtn{ "0" };
    juce::TextButton pitchUp1Btn{ "+1" };
    juce::TextButton pitchUp2Btn{ "+2" };
    juce::TextButton pitchUp3Btn{ "+3" };
    int currentPitchShift{ 0 };
    juce::String currentUrl{ "https://www.youtube.com/results?search_query=karaoke+viet+nam+beat+chuan" };

    // Native Web View Browser Component
    std::unique_ptr<juce::WebBrowserComponent> webBrowser;

    void executePitchShift(int semitones);
    void injectAdSkipScript();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(YouTubePlayerOverlay)
};
