#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_gui_extra/juce_gui_extra.h>
#include "../songbook/SongbookManager.h"
#include "../audio/KeyDetector.h"

class SongbookOverlay : public juce::Component, public juce::ListBoxModel, private juce::TextEditor::Listener
{
public:
    SongbookOverlay(SongbookManager& songbookMgr);
    ~SongbookOverlay() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void visibilityChanged() override;

    // ListBoxModel overrides
    int getNumRows() override;
    void paintListBoxItem(int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected) override;
    void listBoxItemClicked(int row, const juce::MouseEvent&) override;
    void listBoxItemDoubleClicked(int row, const juce::MouseEvent&) override;

    // TextEditor::Listener
    void textEditorTextChanged(juce::TextEditor&) override;

    std::function<void()> onCloseClicked;
    std::function<void(int rootNote, KeyDetector::ScaleType scale, const juce::String& songName)> onApplyTone;

    void refreshList();
    void selectSong(int index);
    void applySelectedTone(const juce::String& toneStr);

private:
    SongbookManager& songbookManager;
    std::vector<SongItem> displayedSongs;
    int selectedIndex{ -1 };
    int semitoneOffset{ 0 };

    // Header Controls
    juce::Label titleLabel;
    juce::TextButton closeButton{ "✕" };

    // Search & Filter
    juce::TextEditor searchEditor;
    juce::ComboBox genreFilterCombo;
    juce::TextButton addSongButton{ juce::String::fromUTF8(u8"➕ Thêm Bài") };
    juce::TextButton importButton{ juce::String::fromUTF8(u8"📥 Nhập JSON") };
    juce::TextButton exportButton{ juce::String::fromUTF8(u8"📤 Xuất JSON") };

    // List View
    juce::ListBox songListBox;

    // Detail Panel Controls
    juce::Label detailTitleLabel;
    juce::Label detailArtistLabel;
    juce::Label detailInfoLabel;

    juce::TextButton maleToneButton;
    juce::TextButton femaleToneButton;
    juce::TextButton origToneButton;

    juce::TextButton transposeDownBtn{ "-1" };
    juce::Label transposeDisplayLabel{ "0 (Gốc)" };
    juce::TextButton transposeUpBtn{ "+1" };

    juce::TextButton applyAutoTuneButton{ juce::String::fromUTF8(u8"⚡ ĐỒNG BỘ VÀO AUTO-TUNE") };
    juce::TextButton saveCustomToneButton{ juce::String::fromUTF8(u8"💾 Lưu Tone Của Tôi") };
    juce::TextButton favoriteButton{ juce::String::fromUTF8(u8"❤️ Yêu Thích") };
    juce::TextButton deleteSongButton{ juce::String::fromUTF8(u8"🗑 Xóa Bài") };

    // Toast notification component (floats on top of all child components)
    class ToastComponent : public juce::Component
    {
    public:
        ToastComponent()
        {
            setAlwaysOnTop(true);
            setInterceptsMouseClicks(false, false);
        }

        void setMessage(const juce::String& msg)
        {
            message = msg;
            repaint();
        }

        void paint(juce::Graphics& g) override
        {
            auto bounds = getLocalBounds().toFloat();
            if (bounds.isEmpty())
                return;

            // Shadow
            g.setColour(juce::Colour(0x99000000));
            g.fillRoundedRectangle(bounds.translated(0, 3), 8.0f);

            // Toast background gradient
            juce::ColourGradient grad(
                juce::Colour(0xf8065f46), bounds.getX(), bounds.getY(),
                juce::Colour(0xf8064e3b), bounds.getX(), bounds.getBottom(),
                false
            );
            g.setGradientFill(grad);
            g.fillRoundedRectangle(bounds, 8.0f);

            // Border
            g.setColour(juce::Colour(0xff34d399));
            g.drawRoundedRectangle(bounds.reduced(0.75f), 8.0f, 1.5f);

            // Text
            g.setColour(juce::Colours::white);
            g.setFont(juce::FontOptions(13.5f, juce::Font::bold));
            g.drawText(message, getLocalBounds().reduced(16, 0), juce::Justification::centred, true);
        }

    private:
        juce::String message;
    };

    ToastComponent toastComponent;
    void showToast(const juce::String& msg);

    juce::String transposeTone(const juce::String& baseTone, int semitones);
    void updateDetailPanel();
    void handleAddSongDialog();

    std::unique_ptr<juce::FileChooser> fileChooser;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SongbookOverlay)
};
