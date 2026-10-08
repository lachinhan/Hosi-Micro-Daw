#include "SongbookOverlay.h"

SongbookOverlay::SongbookOverlay(SongbookManager& songbookMgr, VocalRangeDetector& detector)
    : songbookManager(songbookMgr), vocalRangeDetector(detector)
{
    // Title
    titleLabel.setText(juce::String::fromUTF8(u8"🎵 SỔ TONE BÀI HÁT & GỢI Ý AI (SMART SONGBOOK)"), juce::dontSendNotification);
    titleLabel.setFont(juce::FontOptions(18.0f, juce::Font::bold));
    titleLabel.setColour(juce::Label::textColourId, juce::Colour(0xff38bdf8));
    addAndMakeVisible(titleLabel);

    // Close Button
    closeButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0x33ffffff));
    closeButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    closeButton.onClick = [this]() {
        if (onCloseClicked)
            onCloseClicked();
    };
    addAndMakeVisible(closeButton);

    // Search Editor
    searchEditor.setTextToShowWhenEmpty(juce::String::fromUTF8(u8"🔍 Gõ tên bài hát, ca sĩ, nhạc sĩ (ví dụ: hoa no, hoai lam)..."), juce::Colour(0xff94a3b8));
    searchEditor.setColour(juce::TextEditor::backgroundColourId, juce::Colour(0xff1e293b));
    searchEditor.setColour(juce::TextEditor::textColourId, juce::Colours::white);
    searchEditor.setColour(juce::TextEditor::outlineColourId, juce::Colour(0xff475569));
    searchEditor.setColour(juce::TextEditor::focusedOutlineColourId, juce::Colour(0xff38bdf8));
    searchEditor.addListener(this);
    addAndMakeVisible(searchEditor);

    // Genre Filter
    genreFilterCombo.addItem(juce::String::fromUTF8(u8"🌟 Tất Cả Bài Hát"), 1);
    genreFilterCombo.addItem(juce::String::fromUTF8(u8"🎯 Gợi Ý Vừa Giọng AI"), 100);
    genreFilterCombo.addItem(juce::String::fromUTF8(u8"❤️ Bài Hát Yêu Thích"), 2);
    genreFilterCombo.addItem(juce::String::fromUTF8(u8"🔥 Nhạc Trẻ / Pop"), 3);
    genreFilterCombo.addItem(juce::String::fromUTF8(u8"🎸 Bolero / Nhạc Vàng"), 4);
    genreFilterCombo.addItem(juce::String::fromUTF8(u8"☕ Nhạc Trịnh"), 5);
    genreFilterCombo.addItem(juce::String::fromUTF8(u8"🎼 Trữ Tình / Ballad"), 6);
    genreFilterCombo.addItem(juce::String::fromUTF8(u8"⭐ Bài Tự Thêm"), 7);
    genreFilterCombo.setSelectedId(1, juce::dontSendNotification);
    genreFilterCombo.onChange = [this]() { refreshList(); };
    genreFilterCombo.setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xff1e293b));
    genreFilterCombo.setColour(juce::ComboBox::textColourId, juce::Colours::white);
    genreFilterCombo.setColour(juce::ComboBox::outlineColourId, juce::Colour(0xff475569));
    addAndMakeVisible(genreFilterCombo);

    // AI Vocal Range Button
    aiRangeButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff0284c7)); // Sky Blue
    aiRangeButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    aiRangeButton.onClick = [this]() {
        if (onOpenVocalRangeDetector)
            onOpenVocalRangeDetector();
    };
    addAndMakeVisible(aiRangeButton);

    // Top action buttons
    addSongButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff059669));
    addSongButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    addSongButton.onClick = [this]() { handleAddSongDialog(); };
    addAndMakeVisible(addSongButton);

    importButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff334155));
    importButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    importButton.onClick = [this]() {
        fileChooser = std::make_unique<juce::FileChooser>(
            juce::String::fromUTF8(u8"Chọn file JSON danh sách bài hát"),
            juce::File::getSpecialLocation(juce::File::userDocumentsDirectory),
            "*.json"
        );
        fileChooser->launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
            [this](const juce::FileChooser& fc) {

                auto file = fc.getResult();
                if (file.existsAsFile())
                {
                    if (songbookManager.importFromJson(file))
                    {
                        refreshList();
                        showToast(juce::String::fromUTF8(u8"Đã nhập dữ liệu bài hát thành công!"));
                    }
                }
            });
    };
    addAndMakeVisible(importButton);

    exportButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff334155));
    exportButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    exportButton.onClick = [this]() {
        fileChooser = std::make_unique<juce::FileChooser>(
            juce::String::fromUTF8(u8"Lưu file JSON danh sách bài hát"),
            juce::File::getSpecialLocation(juce::File::userDocumentsDirectory).getChildFile("songbook_export.json"),
            "*.json"
        );
        fileChooser->launchAsync(juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles | juce::FileBrowserComponent::warnAboutOverwriting,
            [this](const juce::FileChooser& fc) {
                auto file = fc.getResult();
                if (file != juce::File{})
                {
                    if (songbookManager.exportToJson(file))
                    {
                        showToast(juce::String::fromUTF8(u8"Đã xuất file JSON thành công!"));
                    }
                }
            });
    };
    addAndMakeVisible(exportButton);

    // ListBox
    songListBox.setModel(this);
    songListBox.setRowHeight(44);
    songListBox.setColour(juce::ListBox::backgroundColourId, juce::Colour(0xff0f172a));
    songListBox.setColour(juce::ListBox::outlineColourId, juce::Colour(0xff334155));
    addAndMakeVisible(songListBox);

    // Detail Panel
    detailTitleLabel.setText(juce::String::fromUTF8(u8"Chọn một bài hát để xem chi tiết"), juce::dontSendNotification);
    detailTitleLabel.setFont(juce::FontOptions(20.0f, juce::Font::bold));
    detailTitleLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(detailTitleLabel);

    detailArtistLabel.setText("", juce::dontSendNotification);
    detailArtistLabel.setFont(juce::FontOptions(14.0f, juce::Font::plain));
    detailArtistLabel.setColour(juce::Label::textColourId, juce::Colour(0xff94a3b8));
    addAndMakeVisible(detailArtistLabel);

    detailInfoLabel.setText("", juce::dontSendNotification);
    detailInfoLabel.setFont(juce::FontOptions(13.0f, juce::Font::plain));
    detailInfoLabel.setColour(juce::Label::textColourId, juce::Colour(0xffcbd5e1));
    addAndMakeVisible(detailInfoLabel);

    // AI Match Banner
    aiMatchBanner.setFont(juce::FontOptions(12.5f, juce::Font::bold));
    aiMatchBanner.setColour(juce::Label::textColourId, juce::Colour(0xff34d399));
    addAndMakeVisible(aiMatchBanner);

    applyAiToneButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff0d9488)); // Teal
    applyAiToneButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    applyAiToneButton.onClick = [this]() {
        if (selectedIndex >= 0 && selectedIndex < static_cast<int>(displayedFits.size()))
        {
            const auto& fit = displayedFits[static_cast<size_t>(selectedIndex)];
            semitoneOffset = fit.recommendedShift;
            applySelectedTone(fit.recommendedTone);
        }
    };
    addAndMakeVisible(applyAiToneButton);

    // Tone Buttons

    maleToneButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff1d4ed8));
    maleToneButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    maleToneButton.onClick = [this]() {
        if (selectedIndex >= 0 && selectedIndex < static_cast<int>(displayedSongs.size()))
        {
            const auto& s = displayedSongs[static_cast<size_t>(selectedIndex)];
            semitoneOffset = 0;
            applySelectedTone(s.keyMale);
        }
    };
    addAndMakeVisible(maleToneButton);

    femaleToneButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xffbe185d));
    femaleToneButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    femaleToneButton.onClick = [this]() {
        if (selectedIndex >= 0 && selectedIndex < static_cast<int>(displayedSongs.size()))
        {
            const auto& s = displayedSongs[static_cast<size_t>(selectedIndex)];
            semitoneOffset = 0;
            applySelectedTone(s.keyFemale);
        }
    };
    addAndMakeVisible(femaleToneButton);

    origToneButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff475569));
    origToneButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    origToneButton.onClick = [this]() {
        if (selectedIndex >= 0 && selectedIndex < static_cast<int>(displayedSongs.size()))
        {
            const auto& s = displayedSongs[static_cast<size_t>(selectedIndex)];
            semitoneOffset = 0;
            applySelectedTone(s.keyOriginal);
        }
    };
    addAndMakeVisible(origToneButton);

    // Transpose
    transposeDownBtn.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff334155));
    transposeDownBtn.onClick = [this]() {
        semitoneOffset--;
        updateDetailPanel();
    };
    addAndMakeVisible(transposeDownBtn);

    transposeDisplayLabel.setJustificationType(juce::Justification::centred);
    transposeDisplayLabel.setFont(juce::FontOptions(14.0f, juce::Font::bold));
    transposeDisplayLabel.setColour(juce::Label::textColourId, juce::Colour(0xff38bdf8));
    addAndMakeVisible(transposeDisplayLabel);

    transposeUpBtn.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff334155));
    transposeUpBtn.onClick = [this]() {
        semitoneOffset++;
        updateDetailPanel();
    };
    addAndMakeVisible(transposeUpBtn);

    // Apply Button
    applyAutoTuneButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff059669));
    applyAutoTuneButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    applyAutoTuneButton.onClick = [this]() {
        if (selectedIndex >= 0 && selectedIndex < static_cast<int>(displayedSongs.size()))
        {
            const auto& s = displayedSongs[static_cast<size_t>(selectedIndex)];
            juce::String baseTone = s.getEffectiveTone();
            juce::String finalTone = transposeTone(baseTone, semitoneOffset);
            applySelectedTone(finalTone);
        }
    };
#if HOSI_PRO_EDITION
    // Open YouTube Beat Button (PRO Edition)
    openYouTubeBeatButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xffe11d48)); // YouTube Crimson
    openYouTubeBeatButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    openYouTubeBeatButton.onClick = [this]() {
        if (selectedIndex >= 0 && selectedIndex < static_cast<int>(displayedSongs.size()))
        {
            const auto& s = displayedSongs[static_cast<size_t>(selectedIndex)];
            if (onPlayYouTubeBeat)
                onPlayYouTubeBeat(s.title + " " + s.artist);
        }
    };
    addAndMakeVisible(openYouTubeBeatButton);
#endif

    // Save Custom Tone
    saveCustomToneButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff0284c7));
    saveCustomToneButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    saveCustomToneButton.onClick = [this]() {
        if (selectedIndex >= 0 && selectedIndex < static_cast<int>(displayedSongs.size()))
        {
            const auto& s = displayedSongs[static_cast<size_t>(selectedIndex)];
            juce::String finalTone = transposeTone(s.getEffectiveTone(), semitoneOffset);
            songbookManager.setCustomTone(s.id, finalTone);
            refreshList();
            showToast(juce::String::fromUTF8(u8"Đã lưu tone riêng [") + finalTone + juce::String::fromUTF8(u8"] cho bài này!"));
        }
    };
    addAndMakeVisible(saveCustomToneButton);

    // Favorite Button
    favoriteButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xffbe123c)); // Crimson red
    favoriteButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    favoriteButton.onClick = [this]() {
        if (selectedIndex >= 0 && selectedIndex < static_cast<int>(displayedSongs.size()))
        {
            const auto& s = displayedSongs[static_cast<size_t>(selectedIndex)];
            bool isFav = songbookManager.toggleFavorite(s.id);
            refreshList();
            showToast(isFav ? juce::String::fromUTF8(u8"❤️ Đã thêm [") + s.title + juce::String::fromUTF8(u8"] vào danh sách Yêu Thích!")
                            : juce::String::fromUTF8(u8"🤍 Đã bỏ [") + s.title + juce::String::fromUTF8(u8"] khỏi danh sách Yêu Thích!"));
        }
    };
    addAndMakeVisible(favoriteButton);

    // Delete Button
    deleteSongButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff991b1b));
    deleteSongButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    deleteSongButton.onClick = [this]() {
        if (selectedIndex >= 0 && selectedIndex < static_cast<int>(displayedSongs.size()))
        {
            const auto& s = displayedSongs[static_cast<size_t>(selectedIndex)];
            songbookManager.deleteSong(s.id);
            refreshList();
            showToast(juce::String::fromUTF8(u8"Đã xóa bài hát!"));
        }
    };
    addAndMakeVisible(deleteSongButton);

    // Toast Component (floats on top)
    addChildComponent(toastComponent);
    toastComponent.setVisible(false);

    // Initial load
    refreshList();
}

void SongbookOverlay::visibilityChanged()
{
    if (isVisible())
    {
        refreshList();
        searchEditor.grabKeyboardFocus();
    }
}

void SongbookOverlay::setFilterToAiMatch()
{
    genreFilterCombo.setSelectedId(100, juce::dontSendNotification);
    refreshList();
}

void SongbookOverlay::refreshList()
{
    juce::String query = searchEditor.getText();
    int filterId = genreFilterCombo.getSelectedId();
    juce::String genreFilter = "ALL";

    if (filterId == 2) genreFilter = "FAVORITES";
    else if (filterId == 3) genreFilter = "NHAC_TRE";
    else if (filterId == 4) genreFilter = "BOLERO";
    else if (filterId == 5) genreFilter = "TRINH";
    else if (filterId == 6) genreFilter = "TRU_TINH";
    else if (filterId == 7) genreFilter = "CUSTOM_USER";

    const auto& prof = vocalRangeDetector.getProfile();
    displayedSongs.clear();
    displayedFits.clear();

    if (filterId == 100) // AI Smart Match Filter
    {
        auto ranked = songbookManager.getAiRecommendedSongs(prof.lowestMidi, prof.highestMidi, query);
        for (const auto& r : ranked)
        {
            displayedSongs.push_back(r.first);
            displayedFits.push_back(r.second);
        }
    }
    else
    {
        displayedSongs = songbookManager.searchSongs(query, genreFilter);
        displayedFits.reserve(displayedSongs.size());
        for (const auto& s : displayedSongs)
        {
            displayedFits.push_back(songbookManager.evaluateSongFit(s, prof.lowestMidi, prof.highestMidi));
        }
    }

    songListBox.updateContent();
    
    if (!displayedSongs.empty())
    {
        if (selectedIndex < 0 || selectedIndex >= static_cast<int>(displayedSongs.size()))
            selectSong(0);
        else
            updateDetailPanel();
    }
    else
    {
        selectedIndex = -1;
        updateDetailPanel();
    }
}

void SongbookOverlay::selectSong(int index)
{
    if (index >= 0 && index < static_cast<int>(displayedSongs.size()))
    {
        selectedIndex = index;
        semitoneOffset = 0;
        songListBox.selectRow(index);
        updateDetailPanel();
    }
}

int SongbookOverlay::getNumRows()
{
    return static_cast<int>(displayedSongs.size());
}

void SongbookOverlay::paintListBoxItem(int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected)
{
    if (rowNumber < 0 || rowNumber >= static_cast<int>(displayedSongs.size()))
        return;

    const auto& item = displayedSongs[static_cast<size_t>(rowNumber)];

    // Background
    if (rowIsSelected)
    {
        g.setColour(juce::Colour(0xff1e3a8a));
        g.fillRoundedRectangle(2.0f, 2.0f, static_cast<float>(width - 4), static_cast<float>(height - 4), 4.0f);
        g.setColour(juce::Colour(0xff38bdf8));
        g.drawRoundedRectangle(2.0f, 2.0f, static_cast<float>(width - 4), static_cast<float>(height - 4), 4.0f, 1.0f);
    }
    else if (rowNumber % 2 == 1)
    {
        g.setColour(juce::Colour(0xff131c2e));
        g.fillRoundedRectangle(2.0f, 2.0f, static_cast<float>(width - 4), static_cast<float>(height - 4), 4.0f);
    }

    // Favorite Heart Icon on the left
    if (item.isFavorite)
    {
        g.setFont(juce::FontOptions(13.0f, juce::Font::plain));
        g.drawText(juce::String::fromUTF8(u8"❤️"), 4, (height - 20) / 2, 22, 20, juce::Justification::centred, false);
    }
    else
    {
        g.setFont(juce::FontOptions(12.0f, juce::Font::plain));
        g.setColour(juce::Colour(0x6664748b));
        g.drawText(juce::String::fromUTF8(u8"🤍"), 4, (height - 20) / 2, 22, 20, juce::Justification::centred, false);
    }

    // Song Title
    g.setFont(juce::FontOptions(14.0f, juce::Font::bold));
    g.setColour(rowIsSelected ? juce::Colours::white : juce::Colour(0xffe2e8f0));
    g.drawText(item.title, 30, 4, width - 235, 18, juce::Justification::left, true);

    // Artist & Genre
    g.setFont(juce::FontOptions(12.0f, juce::Font::plain));
    g.setColour(rowIsSelected ? juce::Colour(0xff93c5fd) : juce::Colour(0xff94a3b8));
    juce::String subtitle = item.artist + "  •  " + item.genre;
    g.drawText(subtitle, 30, 22, width - 235, 16, juce::Justification::left, true);

    // Tone Badges
    int badgeRight = width - 10;
    int badgeW = 52;
    int badgeH = 24;
    int badgeY = (height - badgeH) / 2;

    int filterId = genreFilterCombo.getSelectedId();
    if (filterId == 100 && rowNumber < static_cast<int>(displayedFits.size()))
    {
        // Show AI Match Badge on the right
        const auto& fit = displayedFits[static_cast<size_t>(rowNumber)];
        int aiBadgeW = 95;
        badgeRight -= aiBadgeW;

        juce::Colour badgeCol = (fit.fitScore >= 95) ? juce::Colour(0x33059669) : juce::Colour(0x330284c7);
        juce::Colour textCol = (fit.fitScore >= 95) ? juce::Colour(0xff34d399) : juce::Colour(0xff38bdf8);

        g.setColour(badgeCol);
        g.fillRoundedRectangle(static_cast<float>(badgeRight), static_cast<float>(badgeY), static_cast<float>(aiBadgeW), static_cast<float>(badgeH), 3.0f);
        g.setColour(textCol);
        g.setFont(juce::FontOptions(11.0f, juce::Font::bold));
        g.drawText(fit.fitBadge, badgeRight, badgeY, aiBadgeW, badgeH, juce::Justification::centred, false);
    }
    else
    {
        // Female Tone Badge (Pink)
        badgeRight -= badgeW;
        g.setColour(juce::Colour(0x33db2777));
        g.fillRoundedRectangle(static_cast<float>(badgeRight), static_cast<float>(badgeY), static_cast<float>(badgeW), static_cast<float>(badgeH), 3.0f);
        g.setColour(juce::Colour(0xfff472b6));
        g.setFont(juce::FontOptions(11.0f, juce::Font::bold));
        g.drawText(juce::String::fromUTF8(u8"Nữ: ") + item.keyFemale, badgeRight, badgeY, badgeW, badgeH, juce::Justification::centred, false);

        // Male Tone Badge (Blue)
        badgeRight -= (badgeW + 6);
        g.setColour(juce::Colour(0x332563eb));
        g.fillRoundedRectangle(static_cast<float>(badgeRight), static_cast<float>(badgeY), static_cast<float>(badgeW), static_cast<float>(badgeH), 3.0f);
        g.setColour(juce::Colour(0xff60a5fa));
        g.setFont(juce::FontOptions(11.0f, juce::Font::bold));
        g.drawText(juce::String::fromUTF8(u8"Nam: ") + item.keyMale, badgeRight, badgeY, badgeW, badgeH, juce::Justification::centred, false);
    }

    // Main Tone Badge (Green or Gold if Custom)
    badgeRight -= (badgeW + 6);
    juce::String toneDisplay = item.getEffectiveTone();
    bool hasCustom = item.customKey.isNotEmpty();
    g.setColour(hasCustom ? juce::Colour(0x33d97706) : juce::Colour(0x33059669));
    g.fillRoundedRectangle(static_cast<float>(badgeRight), static_cast<float>(badgeY), static_cast<float>(badgeW), static_cast<float>(badgeH), 3.0f);
    g.setColour(hasCustom ? juce::Colour(0xfffbbf24) : juce::Colour(0xff34d399));
    g.setFont(juce::FontOptions(11.0f, juce::Font::bold));
    g.drawText(toneDisplay, badgeRight, badgeY, badgeW, badgeH, juce::Justification::centred, false);
}

void SongbookOverlay::listBoxItemClicked(int row, const juce::MouseEvent& e)
{
    if (e.x < 30)
    {
        if (row >= 0 && row < static_cast<int>(displayedSongs.size()))
        {
            const auto& s = displayedSongs[static_cast<size_t>(row)];
            bool isFav = songbookManager.toggleFavorite(s.id);
            refreshList();
            showToast(isFav ? juce::String::fromUTF8(u8"❤️ Đã thêm [") + s.title + juce::String::fromUTF8(u8"] vào danh sách Yêu Thích!")
                            : juce::String::fromUTF8(u8"🤍 Đã bỏ [") + s.title + juce::String::fromUTF8(u8"] khỏi danh sách Yêu Thích!"));
            return;
        }
    }
    selectSong(row);
}

void SongbookOverlay::listBoxItemDoubleClicked(int row, const juce::MouseEvent&)
{
    selectSong(row);
    if (selectedIndex >= 0 && selectedIndex < static_cast<int>(displayedSongs.size()))
    {
        const auto& s = displayedSongs[static_cast<size_t>(selectedIndex)];
        applySelectedTone(s.getEffectiveTone());
    }
}

void SongbookOverlay::textEditorTextChanged(juce::TextEditor&)
{
    refreshList();
}

juce::String SongbookOverlay::transposeTone(const juce::String& baseTone, int semitones)
{
    if (semitones == 0)
        return baseTone;

    int root = 0;
    bool isMinor = false;
    SongbookManager::parseKeyAndScale(baseTone, root, isMinor);

    int newRoot = (root + semitones) % 12;
    if (newRoot < 0) newRoot += 12;

    return KeyDetector::formatKeyName(newRoot, isMinor ? KeyDetector::ScaleType::Minor : KeyDetector::ScaleType::Major);
}

void SongbookOverlay::updateDetailPanel()
{
    if (selectedIndex < 0 || selectedIndex >= static_cast<int>(displayedSongs.size()))
    {
        detailTitleLabel.setText(juce::String::fromUTF8(u8"Không tìm thấy bài hát phù hợp"), juce::dontSendNotification);
        detailArtistLabel.setText("", juce::dontSendNotification);
        detailInfoLabel.setText("", juce::dontSendNotification);
        aiMatchBanner.setVisible(false);
        applyAiToneButton.setVisible(false);
        maleToneButton.setVisible(false);
        femaleToneButton.setVisible(false);
        origToneButton.setVisible(false);
        applyAutoTuneButton.setEnabled(false);
        saveCustomToneButton.setEnabled(false);
        favoriteButton.setVisible(false);
        deleteSongButton.setVisible(false);
        return;
    }

    const auto& song = displayedSongs[static_cast<size_t>(selectedIndex)];

    detailTitleLabel.setText(song.title, juce::dontSendNotification);
    detailArtistLabel.setText(juce::String::fromUTF8(u8"Ca sĩ: ") + song.artist + juce::String::fromUTF8(u8"  •  Sáng tác: ") + song.composer, juce::dontSendNotification);
    
    juce::String info = juce::String::fromUTF8(u8"Thể loại: ") + song.genre +
                        juce::String::fromUTF8(u8"   |   Scale: ") + song.scale +
                        juce::String::fromUTF8(u8"   |   Tempo: ~") + juce::String(song.tempo) + " BPM";
    if (song.customKey.isNotEmpty())
        info += juce::String::fromUTF8(u8"   |   ⭐ Tone riêng: ") + song.customKey;
    detailInfoLabel.setText(info, juce::dontSendNotification);

    // AI Fit info
    if (selectedIndex < static_cast<int>(displayedFits.size()))
    {
        const auto& fit = displayedFits[static_cast<size_t>(selectedIndex)];
        aiMatchBanner.setVisible(true);
        aiMatchBanner.setText(juce::String::fromUTF8(u8"✨ AI Match: ") + fit.fitBadge + " • " + fit.advice, juce::dontSendNotification);
        
        applyAiToneButton.setVisible(true);
        applyAiToneButton.setButtonText(juce::String::fromUTF8(u8"✨ DÙNG TONE AI GỢI Ý [") + fit.recommendedTone + "]");
    }
    else
    {
        aiMatchBanner.setVisible(false);
        applyAiToneButton.setVisible(false);
    }

    maleToneButton.setVisible(true);
    maleToneButton.setButtonText(juce::String::fromUTF8(u8"👨 TONE NAM: ") + song.keyMale);

    femaleToneButton.setVisible(true);
    femaleToneButton.setButtonText(juce::String::fromUTF8(u8"👩 TONE NỮ: ") + song.keyFemale);

    origToneButton.setVisible(true);
    origToneButton.setButtonText(juce::String::fromUTF8(u8"🎵 TONE GỐC: ") + song.keyOriginal);

    // Transpose text
    juce::String currentTransposed = transposeTone(song.getEffectiveTone(), semitoneOffset);
    if (semitoneOffset == 0)
        transposeDisplayLabel.setText(currentTransposed + juce::String::fromUTF8(u8" (Chuẩn)"), juce::dontSendNotification);
    else if (semitoneOffset > 0)
        transposeDisplayLabel.setText(currentTransposed + " (+" + juce::String(semitoneOffset) + ")", juce::dontSendNotification);
    else
        transposeDisplayLabel.setText(currentTransposed + " (" + juce::String(semitoneOffset) + ")", juce::dontSendNotification);

    applyAutoTuneButton.setEnabled(true);
    applyAutoTuneButton.setButtonText(juce::String::fromUTF8(u8"⚡ ĐỒNG BỘ [") + currentTransposed + juce::String::fromUTF8(u8"] VÀO AUTO-TUNE"));

    saveCustomToneButton.setEnabled(true);

    if (song.isFavorite)
    {
        favoriteButton.setButtonText(juce::String::fromUTF8(u8"❤️ ĐÃ YÊU THÍCH"));
        favoriteButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xffbe123c)); // Rose red
        favoriteButton.setColour(juce::TextButton::textColourOffId, juce::Colour(0xffffe4e6));
    }
    else
    {
        favoriteButton.setButtonText(juce::String::fromUTF8(u8"🤍 THÊM YÊU THÍCH"));
        favoriteButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff334155)); // Slate
        favoriteButton.setColour(juce::TextButton::textColourOffId, juce::Colour(0xffcbd5e1));
    }
    favoriteButton.setVisible(true);

    deleteSongButton.setVisible(song.isCustom);
}


void SongbookOverlay::applySelectedTone(const juce::String& toneStr)
{
    if (selectedIndex < 0 || selectedIndex >= static_cast<int>(displayedSongs.size()))
        return;

    const auto& song = displayedSongs[static_cast<size_t>(selectedIndex)];
    int root = 0;
    bool isMinor = false;
    SongbookManager::parseKeyAndScale(toneStr, root, isMinor);

    if (onApplyTone)
    {
        onApplyTone(root, isMinor ? KeyDetector::ScaleType::Minor : KeyDetector::ScaleType::Major, song.title);
    }

    if (onApplyTempo && song.tempo > 0)
    {
        onApplyTempo(static_cast<double>(song.tempo), song.title);
    }

    showToast(juce::String::fromUTF8(u8"Đã đồng bộ Tone [") + toneStr + juce::String::fromUTF8(u8"] (Tempo: ") + juce::String(song.tempo) + juce::String::fromUTF8(u8" BPM) bài [") + song.title + juce::String::fromUTF8(u8"] vào Auto-Tune!"));
}

void SongbookOverlay::showToast(const juce::String& msg)
{
    toastComponent.setMessage(msg);

    // Compute appropriate width based on text length with safety margins
    juce::Font font(juce::FontOptions(13.5f, juce::Font::bold));
    int textWidth = font.getStringWidth(msg);
    int maxToastW = getWidth() - 80;
    int toastW = std::clamp(textWidth + 56, 360, maxToastW > 360 ? maxToastW : 360);
    int toastH = 42;
    int toastX = (getWidth() - toastW) / 2;
    int toastY = getHeight() - 56;

    toastComponent.setBounds(toastX, toastY, toastW, toastH);
    toastComponent.toFront(true);
    toastComponent.setVisible(true);

    juce::Timer::callAfterDelay(3200, [safe = juce::Component::SafePointer<SongbookOverlay>(this)]() {
        if (safe != nullptr)
        {
            safe->toastComponent.setVisible(false);
        }
    });
}

void SongbookOverlay::handleAddSongDialog()
{
    auto* dialog = new juce::AlertWindow(
        juce::String::fromUTF8(u8"Thêm Bài Hát Mới Vào Sổ Tone"),
        juce::String::fromUTF8(u8"Nhập thông tin bài hát và tone nhạc:"),
        juce::AlertWindow::QuestionIcon
    );

    dialog->addTextEditor("title", "", juce::String::fromUTF8(u8"Tên bài hát:"));
    dialog->addTextEditor("artist", "", juce::String::fromUTF8(u8"Ca sĩ:"));
    dialog->addTextEditor("key_orig", "Am", juce::String::fromUTF8(u8"Tone gốc (vd: Am, C, F#m):"));
    dialog->addTextEditor("key_male", "Am", juce::String::fromUTF8(u8"Tone Nam (vd: Am, Em):"));
    dialog->addTextEditor("key_female", "Dm", juce::String::fromUTF8(u8"Tone Nữ (vd: Dm, Am):"));
    dialog->addTextEditor("genre", "Ballad", juce::String::fromUTF8(u8"Thể loại (vd: Ballad, Bolero, Pop):"));

    dialog->addButton(juce::String::fromUTF8(u8"Lưu Bài"), 1, juce::KeyPress(juce::KeyPress::returnKey));
    dialog->addButton(juce::String::fromUTF8(u8"Hủy"), 0, juce::KeyPress(juce::KeyPress::escapeKey));

    dialog->enterModalState(true, juce::ModalCallbackFunction::create([this, dialog](int result) {
        if (result == 1)
        {
            SongItem newItem;
            newItem.title = dialog->getTextEditorContents("title").trim();
            newItem.artist = dialog->getTextEditorContents("artist").trim();
            newItem.keyOriginal = dialog->getTextEditorContents("key_orig").trim();
            newItem.keyMale = dialog->getTextEditorContents("key_male").trim();
            newItem.keyFemale = dialog->getTextEditorContents("key_female").trim();
            newItem.genre = dialog->getTextEditorContents("genre").trim();

            if (newItem.title.isNotEmpty())
            {
                songbookManager.addSong(newItem);
                refreshList();
                showToast(juce::String::fromUTF8(u8"Đã thêm bài hát [") + newItem.title + juce::String::fromUTF8(u8"] thành công!"));
            }
        }
        delete dialog;
    }));
}

void SongbookOverlay::paint(juce::Graphics& g)
{
    // Semi-transparent backdrop blur
    g.fillAll(juce::Colour(0xdd0b0f19));

    // Modal Card Window
    auto bounds = getLocalBounds().reduced(30, 20);
    g.setColour(juce::Colour(0xff0f172a));
    g.fillRoundedRectangle(bounds.toFloat(), 12.0f);
    g.setColour(juce::Colour(0xff334155));
    g.drawRoundedRectangle(bounds.toFloat(), 12.0f, 1.5f);

    // Detail Panel Background
    int detailLeft = bounds.getX() + static_cast<int>(bounds.getWidth() * 0.58f);
    int detailTop = bounds.getY() + 90;
    int detailW = bounds.getRight() - detailLeft - 16;
    int detailH = bounds.getBottom() - detailTop - 16;

    g.setColour(juce::Colour(0xff1e293b));
    g.fillRoundedRectangle(static_cast<float>(detailLeft), static_cast<float>(detailTop), static_cast<float>(detailW), static_cast<float>(detailH), 8.0f);
    g.setColour(juce::Colour(0xff475569));
    g.drawRoundedRectangle(static_cast<float>(detailLeft), static_cast<float>(detailTop), static_cast<float>(detailW), static_cast<float>(detailH), 8.0f, 1.0f);

    // Section header line in detail panel
    g.setColour(juce::Colour(0xff334155));
    g.drawHorizontalLine(detailTop + 100, static_cast<float>(detailLeft + 12), static_cast<float>(detailLeft + detailW - 12));
}

void SongbookOverlay::resized()
{
    auto bounds = getLocalBounds().reduced(30, 20);

    // Top Header
    auto headerArea = bounds.removeFromTop(44);
    closeButton.setBounds(headerArea.removeFromRight(36).reduced(4, 4));
    titleLabel.setBounds(headerArea.removeFromLeft(520).reduced(10, 4));

    // Filter Bar
    auto filterBar = bounds.removeFromTop(38).reduced(12, 0);
    searchEditor.setBounds(filterBar.removeFromLeft(280));
    filterBar.removeFromLeft(8);
    genreFilterCombo.setBounds(filterBar.removeFromLeft(165));
    filterBar.removeFromLeft(8);
    aiRangeButton.setBounds(filterBar.removeFromLeft(130));
    filterBar.removeFromLeft(8);
    addSongButton.setBounds(filterBar.removeFromLeft(95));
    filterBar.removeFromLeft(6);
    importButton.setBounds(filterBar.removeFromLeft(70));
    filterBar.removeFromLeft(6);
    exportButton.setBounds(filterBar.removeFromLeft(70));

    bounds.removeFromTop(10);

    // Left ListBox vs Right Detail
    int listWidth = static_cast<int>(bounds.getWidth() * 0.55f);
    auto listArea = bounds.removeFromLeft(listWidth).reduced(12, 10);
    songListBox.setBounds(listArea);

    bounds.removeFromLeft(16);
    auto detailArea = bounds.reduced(16, 12);

    // Detail Components
    detailTitleLabel.setBounds(detailArea.removeFromTop(30));
    detailArtistLabel.setBounds(detailArea.removeFromTop(22));
    detailInfoLabel.setBounds(detailArea.removeFromTop(22));
    detailArea.removeFromTop(6);

    // AI Match Banner & Quick Apply
    if (aiMatchBanner.isVisible())
    {
        aiMatchBanner.setBounds(detailArea.removeFromTop(24));
        detailArea.removeFromTop(4);
        applyAiToneButton.setBounds(detailArea.removeFromTop(32));
        detailArea.removeFromTop(10);
    }
    else
    {
        detailArea.removeFromTop(10);
    }

    // Quick Tone Buttons Row
    auto toneButtonsRow = detailArea.removeFromTop(36);
    int btnW = (toneButtonsRow.getWidth() - 16) / 3;
    maleToneButton.setBounds(toneButtonsRow.removeFromLeft(btnW));
    toneButtonsRow.removeFromLeft(8);
    femaleToneButton.setBounds(toneButtonsRow.removeFromLeft(btnW));
    toneButtonsRow.removeFromLeft(8);
    origToneButton.setBounds(toneButtonsRow.removeFromLeft(btnW));

    detailArea.removeFromTop(12);

    // Transpose row
    auto transposeRow = detailArea.removeFromTop(32);
    transposeDownBtn.setBounds(transposeRow.removeFromLeft(56));
    transposeUpBtn.setBounds(transposeRow.removeFromRight(56));
    transposeDisplayLabel.setBounds(transposeRow);

    detailArea.removeFromTop(14);

    // Big Action Button
    applyAutoTuneButton.setBounds(detailArea.removeFromTop(38));
    detailArea.removeFromTop(8);
#if HOSI_PRO_EDITION
    openYouTubeBeatButton.setBounds(detailArea.removeFromTop(34));
    detailArea.removeFromTop(8);
#endif

    // Save Custom Tone & Favorite Row
    auto actionRow = detailArea.removeFromTop(32);
    int halfW = (actionRow.getWidth() - 8) / 2;
    saveCustomToneButton.setBounds(actionRow.removeFromLeft(halfW));
    favoriteButton.setBounds(actionRow.removeFromRight(halfW));
    detailArea.removeFromTop(8);

    deleteSongButton.setBounds(detailArea.removeFromTop(28).removeFromRight(120));

    // Toast positioning if visible
    if (toastComponent.isVisible())
    {
        int toastW = toastComponent.getWidth();
        int toastH = toastComponent.getHeight();
        toastComponent.setBounds((getWidth() - toastW) / 2, getHeight() - 56, toastW, toastH);
        toastComponent.toFront(true);
    }
}

