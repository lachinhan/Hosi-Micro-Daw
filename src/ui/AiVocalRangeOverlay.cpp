#include "AiVocalRangeOverlay.h"
#include <algorithm>

AiVocalRangeOverlay::AiVocalRangeOverlay(VocalRangeDetector& detector, SongbookManager& songbookMgr)
    : rangeDetector(detector),
      songbookManager(songbookMgr),
      rangeBarVisualizer(detector),
      scanProgressBar(progressValue)
{
    // Title
    titleLabel.setText(juce::String::fromUTF8(u8"🎙️ NHẬN DIỆN ÂM VỰC & GỢI Ý BÀI HÁT (SMART VOCAL MATCH)"), juce::dontSendNotification);
    titleLabel.setFont(juce::FontOptions(17.0f, juce::Font::bold));
    titleLabel.setColour(juce::Label::textColourId, juce::Colour(0xff38bdf8)); // Cyan
    addAndMakeVisible(titleLabel);

    subtitleLabel.setText(juce::String::fromUTF8(u8"AI tự động đo đạc quãng giọng của bạn và đề xuất các bài hát vừa vặn nhất, không bị với nốt cao hay tịt nốt trầm."), juce::dontSendNotification);
    subtitleLabel.setFont(juce::FontOptions(12.5f, juce::Font::plain));
    subtitleLabel.setColour(juce::Label::textColourId, juce::Colour(0xff94a3b8));
    addAndMakeVisible(subtitleLabel);

    // Close Button
    closeButton.setButtonText(juce::String::fromUTF8(u8"✕"));
    closeButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0x33ffffff));
    closeButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    closeButton.onClick = [this]() {
        if (onCloseClicked)
            onCloseClicked();
    };
    addAndMakeVisible(closeButton);


    // Range Bar Visualizer
    addAndMakeVisible(rangeBarVisualizer);

    // Start Scan Button
    startScanButton.setButtonText(juce::String::fromUTF8(u8"🎙️ BẮT ĐẦU ĐO ÂM VỰC (15 GIÂY)"));
    startScanButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff059669)); // Emerald Green
    startScanButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    startScanButton.onClick = [this]() {
        if (rangeDetector.isScanning())
        {
            rangeDetector.stopScan();
            startScanButton.setButtonText(juce::String::fromUTF8(u8"🎙️ BẮT ĐẦU ĐO ÂM VỰC (15 GIÂY)"));
            startScanButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff059669));
            scanStatusLabel.setText(juce::String::fromUTF8(u8"✅ Đã hoàn tất đo âm vực! Kết quả đã được lưu."), juce::dontSendNotification);
            updateRecommendations();
        }
        else
        {
            rangeDetector.startScan(15.0f);
            startScanButton.setButtonText(juce::String::fromUTF8(u8"⏹️ DỪNG ĐO & LƯU KẾT QUẢ"));
            startScanButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xffdc2626)); // Red
            scanStatusLabel.setText(juce::String::fromUTF8(u8"🎙️ Đang quét giọng... Hãy ngân từ nốt trầm nhất (\"Ồ...\") rồi lướt dần lên nốt cao nhất (\"Í...\")!"), juce::dontSendNotification);
        }
    };
    addAndMakeVisible(startScanButton);

    scanStatusLabel.setText(juce::String::fromUTF8(u8"💡 Cách đo chuẩn: Lấy hơi sâu, ngân từ nốt trầm nhất (\"Ồ...\") rồi lướt dần giọng lên nốt cao nhất (\"Í...\") vào Micro."), juce::dontSendNotification);
    scanStatusLabel.setFont(juce::FontOptions(12.5f, juce::Font::plain));
    scanStatusLabel.setColour(juce::Label::textColourId, juce::Colour(0xfffcd34d)); // Amber
    addAndMakeVisible(scanStatusLabel);


    scanProgressBar.setColour(juce::ProgressBar::foregroundColourId, juce::Colour(0xff10b981));
    scanProgressBar.setColour(juce::ProgressBar::backgroundColourId, juce::Colour(0xff1e293b));
    addAndMakeVisible(scanProgressBar);

    // Manual Note Adjusters
    manualLabel.setText(juce::String::fromUTF8(u8"Hoặc chỉnh âm vực thủ công:"), juce::dontSendNotification);
    manualLabel.setFont(juce::FontOptions(12.5f, juce::Font::bold));
    manualLabel.setColour(juce::Label::textColourId, juce::Colour(0xffcbd5e1));
    addAndMakeVisible(manualLabel);

    lowNoteLabel.setFont(juce::FontOptions(12.0f, juce::Font::plain));
    lowNoteLabel.setColour(juce::Label::textColourId, juce::Colour(0xff94a3b8));
    addAndMakeVisible(lowNoteLabel);

    highNoteLabel.setFont(juce::FontOptions(12.0f, juce::Font::plain));
    highNoteLabel.setColour(juce::Label::textColourId, juce::Colour(0xff94a3b8));
    addAndMakeVisible(highNoteLabel);

    setupNoteCombos();
    addAndMakeVisible(lowNoteCombo);
    addAndMakeVisible(highNoteCombo);

    // Vocal Classification Banner
    vocalClassBanner.setFont(juce::FontOptions(14.0f, juce::Font::bold));
    vocalClassBanner.setColour(juce::Label::textColourId, juce::Colours::white);
    vocalClassBanner.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(vocalClassBanner);

    // Recommendation Section Header
    recommendSectionLabel.setText(juce::String::fromUTF8(u8"⭐ TOP BÀI HÁT PHÙ HỢP HOÀN HẢO VỚI QUÃNG GIỌNG CỦA BẠN:"), juce::dontSendNotification);
    recommendSectionLabel.setFont(juce::FontOptions(13.5f, juce::Font::bold));
    recommendSectionLabel.setColour(juce::Label::textColourId, juce::Colour(0xff34d399)); // Emerald 400
    addAndMakeVisible(recommendSectionLabel);

    // Initialize 5 Recommendation Rows
    for (int i = 0; i < 5; ++i)
    {
        SongRow row;
        row.titleLabel = std::make_unique<juce::Label>();
        row.titleLabel->setFont(juce::FontOptions(13.0f, juce::Font::bold));
        row.titleLabel->setColour(juce::Label::textColourId, juce::Colours::white);
        addAndMakeVisible(*row.titleLabel);

        row.infoLabel = std::make_unique<juce::Label>();
        row.infoLabel->setFont(juce::FontOptions(11.5f, juce::Font::plain));
        row.infoLabel->setColour(juce::Label::textColourId, juce::Colour(0xff94a3b8));
        addAndMakeVisible(*row.infoLabel);

        row.badgeLabel = std::make_unique<juce::Label>();
        row.badgeLabel->setFont(juce::FontOptions(11.5f, juce::Font::bold));
        row.badgeLabel->setColour(juce::Label::textColourId, juce::Colour(0xff34d399));
        row.badgeLabel->setJustificationType(juce::Justification::centred);
        addAndMakeVisible(*row.badgeLabel);

        row.singButton = std::make_unique<juce::TextButton>(juce::String::fromUTF8(u8"⚡ Hát Ngay"));
        row.singButton->setColour(juce::TextButton::buttonColourId, juce::Colour(0xff2563eb));
        row.singButton->setColour(juce::TextButton::textColourOffId, juce::Colours::white);
        addAndMakeVisible(*row.singButton);

        songRows.push_back(std::move(row));
    }

    // Bottom Action Buttons
    viewAllInSongbookBtn.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff0284c7)); // Sky Blue
    viewAllInSongbookBtn.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    viewAllInSongbookBtn.onClick = [this]() {
        if (onOpenFullSongbookAi)
            onOpenFullSongbookAi();
    };
    addAndMakeVisible(viewAllInSongbookBtn);

    saveAndCloseBtn.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff059669)); // Emerald
    saveAndCloseBtn.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    saveAndCloseBtn.onClick = [this]() {
        rangeDetector.saveProfileToDisk();
        if (onCloseClicked)
            onCloseClicked();
    };
    addAndMakeVisible(saveAndCloseBtn);

    updateRecommendations();
}

AiVocalRangeOverlay::~AiVocalRangeOverlay()
{
    stopTimer();
}

void AiVocalRangeOverlay::visibilityChanged()
{
    if (isVisible())
    {
        startTimerHz(30); // 30 fps
        updateRecommendations();
    }
    else
    {
        stopTimer();
    }
}

void AiVocalRangeOverlay::setupNoteCombos()
{
    lowNoteCombo.clear();
    highNoteCombo.clear();

    // Notes C2 (36) to C6 (84)
    int id = 1;
    for (int midi = 36; midi <= 84; ++midi)
    {
        juce::String noteName = VocalRangeDetector::midiToNoteName(midi);
        lowNoteCombo.addItem(noteName, id);
        highNoteCombo.addItem(noteName, id);
        id++;
    }

    const auto& prof = rangeDetector.getProfile();
    lowNoteCombo.setSelectedId(prof.lowestMidi - 36 + 1, juce::dontSendNotification);
    highNoteCombo.setSelectedId(prof.highestMidi - 36 + 1, juce::dontSendNotification);

    lowNoteCombo.onChange = [this]() {
        int lowMidi = lowNoteCombo.getSelectedId() - 1 + 36;
        int highMidi = highNoteCombo.getSelectedId() - 1 + 36;
        if (lowMidi < highMidi)
        {
            rangeDetector.setCustomRange(lowMidi, highMidi);
            updateRecommendations();
        }
    };

    highNoteCombo.onChange = [this]() {
        int lowMidi = lowNoteCombo.getSelectedId() - 1 + 36;
        int highMidi = highNoteCombo.getSelectedId() - 1 + 36;
        if (highMidi > lowMidi)
        {
            rangeDetector.setCustomRange(lowMidi, highMidi);
            updateRecommendations();
        }
    };
}

void AiVocalRangeOverlay::timerCallback()
{
    progressValue = rangeDetector.getScanProgress();

    if (rangeDetector.isScanning())
    {
        float remSec = rangeDetector.getRemainingScanSeconds();
        auto live = rangeDetector.getLivePitch();
        const auto& prof = rangeDetector.getProfile();

        juce::String status = juce::String::fromUTF8(u8"🎙️ ĐANG ĐO (còn ") + juce::String(remSec, 1) + juce::String::fromUTF8(u8"s)... Hãy ngân lướt: \"Ồ ➔ Ó ➔ Í...\"!");
        if (live.isVoiceActive && live.currentMidi > 0)
        {
            status += juce::String::fromUTF8(u8" | Nốt đang hát: ") + live.noteName + " (" + juce::String(live.currentHz, 1) + " Hz)";
        }
        scanStatusLabel.setText(status, juce::dontSendNotification);

        // Update Banner text live as the user sings
        juce::String bannerText = juce::String::fromUTF8(u8"🏷️ Phân Loại Giọng: ") + prof.vocalClassName +
            juce::String::fromUTF8(u8"  |  Quãng: ") + prof.getLowestNoteName() + " ➔ " + prof.getHighestNoteName() +
            " (" + juce::String(prof.getSpanSemitones()) + juce::String::fromUTF8(u8" bán âm)");
        vocalClassBanner.setText(bannerText, juce::dontSendNotification);

        // Update combos without triggering events
        lowNoteCombo.setSelectedId(prof.lowestMidi - 36 + 1, juce::dontSendNotification);
        highNoteCombo.setSelectedId(prof.highestMidi - 36 + 1, juce::dontSendNotification);
    }
    else if (startScanButton.getButtonText().contains(juce::String::fromUTF8(u8"DỪNG")))
    {
        startScanButton.setButtonText(juce::String::fromUTF8(u8"🎙️ BẮT ĐẦU ĐO ÂM VỰC (15 GIÂY)"));
        startScanButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff059669));
        scanStatusLabel.setText(juce::String::fromUTF8(u8"✅ Đo âm vực hoàn tất! Bạn có thể đo lại bất cứ lúc nào hoặc bấm 'Lưu & Hoàn Tất'."), juce::dontSendNotification);
        updateRecommendations();
    }

    rangeBarVisualizer.repaint();
}


void AiVocalRangeOverlay::updateRecommendations()
{
    const auto& prof = rangeDetector.getProfile();

    // Update combos without triggering events
    lowNoteCombo.setSelectedId(prof.lowestMidi - 36 + 1, juce::dontSendNotification);
    highNoteCombo.setSelectedId(prof.highestMidi - 36 + 1, juce::dontSendNotification);

    // Update Banner
    juce::String bannerText = juce::String::fromUTF8(u8"🏷️ Phân Loại Giọng: ") + prof.vocalClassName +
        juce::String::fromUTF8(u8"  |  Quãng: ") + prof.getLowestNoteName() + " ➔ " + prof.getHighestNoteName() +
        " (" + juce::String(prof.getSpanSemitones()) + juce::String::fromUTF8(u8" bán âm)");
    vocalClassBanner.setText(bannerText, juce::dontSendNotification);

    // Fetch top AI recommended songs
    auto recommended = songbookManager.getAiRecommendedSongs(prof.lowestMidi, prof.highestMidi);

    for (size_t i = 0; i < songRows.size(); ++i)
    {
        if (i < recommended.size())
        {
            const auto& song = recommended[i].first;
            const auto& fit = recommended[i].second;

            songRows[i].titleLabel->setText(song.title + " - " + song.artist, juce::dontSendNotification);
            songRows[i].titleLabel->setVisible(true);

            juce::String info = fit.advice;
            songRows[i].infoLabel->setText(info, juce::dontSendNotification);
            songRows[i].infoLabel->setVisible(true);

            songRows[i].badgeLabel->setText(fit.fitBadge, juce::dontSendNotification);
            songRows[i].badgeLabel->setVisible(true);

            songRows[i].singButton->setButtonText(juce::String::fromUTF8(u8"⚡ Hát [") + fit.recommendedTone + "]");
            songRows[i].singButton->onClick = [this, song, fit]() {
                applySongRecommendation(song, fit);
            };
            songRows[i].singButton->setVisible(true);
        }
        else
        {
            songRows[i].titleLabel->setVisible(false);
            songRows[i].infoLabel->setVisible(false);
            songRows[i].badgeLabel->setVisible(false);
            songRows[i].singButton->setVisible(false);
        }
    }

    repaint();
}

void AiVocalRangeOverlay::applySongRecommendation(const SongItem& song, const SongbookManager::SongFitResult& fit)
{
    int root = 0;
    bool isMinor = false;
    SongbookManager::parseKeyAndScale(fit.recommendedTone, root, isMinor);

    if (onApplyTone)
        onApplyTone(root, isMinor ? KeyDetector::ScaleType::Minor : KeyDetector::ScaleType::Major, song.title);

    if (onApplyTempo && song.tempo > 0)
        onApplyTempo(static_cast<double>(song.tempo), song.title);

    if (onPlayYouTubeBeat)
        onPlayYouTubeBeat(song.title + " " + song.artist);

    if (onCloseClicked)
        onCloseClicked();
}

void AiVocalRangeOverlay::RangeBarVisualizer::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    if (bounds.isEmpty())
        return;

    // Background piano track
    g.setColour(juce::Colour(0xff1e293b));
    g.fillRoundedRectangle(bounds, 6.0f);
    g.setColour(juce::Colour(0xff334155));
    g.drawRoundedRectangle(bounds, 6.0f, 1.0f);

    const int totalNotes = 48; // C2 (36) to C6 (84)
    const float noteW = bounds.getWidth() / static_cast<float>(totalNotes);

    // Draw Octave grid lines and labels
    for (int i = 0; i <= totalNotes; ++i)
    {
        int midi = 36 + i;
        float x = bounds.getX() + i * noteW;

        if (midi % 12 == 0) // C notes
        {
            g.setColour(juce::Colour(0x66475569));
            g.drawVerticalLine(static_cast<int>(x), bounds.getY(), bounds.getBottom());

            g.setColour(juce::Colour(0xff94a3b8));
            g.setFont(juce::FontOptions(10.0f, juce::Font::bold));
            g.drawText(VocalRangeDetector::midiToNoteName(midi), static_cast<int>(x) - 12, static_cast<int>(bounds.getBottom() - 14), 24, 14, juce::Justification::centred, false);
        }
    }

    const auto& prof = detector.getProfile();
    float lowX = bounds.getX() + (std::clamp(prof.lowestMidi, 36, 84) - 36) * noteW;
    float highX = bounds.getX() + (std::clamp(prof.highestMidi, 36, 84) - 36 + 1) * noteW;
    float spanW = std::max(10.0f, highX - lowX);

    // Draw active user range span
    juce::ColourGradient spanGrad(
        juce::Colour(0x660284c7), lowX, bounds.getY(),
        juce::Colour(0x6610b981), highX, bounds.getY(),
        false
    );
    g.setGradientFill(spanGrad);
    g.fillRoundedRectangle(lowX, bounds.getY() + 4, spanW, bounds.getHeight() - 20, 4.0f);

    g.setColour(juce::Colour(0xff38bdf8));
    g.drawRoundedRectangle(lowX, bounds.getY() + 4, spanW, bounds.getHeight() - 20, 4.0f, 1.5f);

    // Draw Low and High Note Labels above
    g.setFont(juce::FontOptions(11.0f, juce::Font::bold));
    g.setColour(juce::Colour(0xff60a5fa));
    g.drawText(prof.getLowestNoteName(), static_cast<int>(lowX) - 15, static_cast<int>(bounds.getY() + 6), 30, 16, juce::Justification::centred, false);

    g.setColour(juce::Colour(0xff34d399));
    g.drawText(prof.getHighestNoteName(), static_cast<int>(highX) - 15, static_cast<int>(bounds.getY() + 6), 30, 16, juce::Justification::centred, false);

    // Real-time bouncing live pin
    auto live = detector.getLivePitch();
    if (live.isVoiceActive && live.currentMidi >= 36 && live.currentMidi <= 84)
    {
        float liveX = bounds.getX() + (live.currentMidi - 36 + 0.5f) * noteW;

        // Glowing live pin
        g.setColour(juce::Colour(0xffef4444)); // Bright Red/Rose
        g.fillEllipse(liveX - 6.0f, bounds.getY() + (bounds.getHeight() - 20) / 2.0f - 2.0f, 12.0f, 12.0f);
        g.setColour(juce::Colours::white);
        g.drawEllipse(liveX - 6.0f, bounds.getY() + (bounds.getHeight() - 20) / 2.0f - 2.0f, 12.0f, 12.0f, 1.5f);

        // Note tag
        g.setFont(juce::FontOptions(10.5f, juce::Font::bold));
        g.setColour(juce::Colours::white);
        g.drawText(live.noteName, static_cast<int>(liveX) - 20, static_cast<int>(bounds.getY() - 1), 40, 14, juce::Justification::centred, false);
    }
}

void AiVocalRangeOverlay::paint(juce::Graphics& g)
{
    // Backdrop Dim
    g.fillAll(juce::Colour(0xdd0b0f19));

    // Modal Card
    auto bounds = getLocalBounds().reduced(20, 15).toFloat();
    g.setColour(juce::Colour(0xff0f172a));
    g.fillRoundedRectangle(bounds, 12.0f);
    g.setColour(juce::Colour(0xff334155));
    g.drawRoundedRectangle(bounds, 12.0f, 1.5f);

    // Card Banner Background
    auto bannerArea = vocalClassBanner.getBounds().toFloat().expanded(4, 2);
    juce::ColourGradient bGrad(
        juce::Colour(0x331e3a8a), bannerArea.getX(), bannerArea.getY(),
        juce::Colour(0x33065f46), bannerArea.getRight(), bannerArea.getY(),
        false
    );
    g.setGradientFill(bGrad);
    g.fillRoundedRectangle(bannerArea, 6.0f);
    g.setColour(juce::Colour(0xff38bdf8));
    g.drawRoundedRectangle(bannerArea, 6.0f, 1.0f);

    // Recommendation Section Card
    for (const auto& row : songRows)
    {
        if (row.titleLabel->isVisible())
        {
            auto rowBounds = row.titleLabel->getBounds().withRight(row.singButton->getRight() + 4).withHeight(36).toFloat();
            g.setColour(juce::Colour(0xff1e293b));
            g.fillRoundedRectangle(rowBounds, 4.0f);
            g.setColour(juce::Colour(0x33475569));
            g.drawRoundedRectangle(rowBounds, 4.0f, 1.0f);
        }
    }
}

void AiVocalRangeOverlay::resized()
{
    auto bounds = getLocalBounds().reduced(24, 18);

    // Header
    auto headerArea = bounds.removeFromTop(32);
    closeButton.setBounds(headerArea.removeFromRight(30).reduced(2));
    titleLabel.setBounds(headerArea);

    subtitleLabel.setBounds(bounds.removeFromTop(20));
    bounds.removeFromTop(10);

    // Range Bar Visualizer
    rangeBarVisualizer.setBounds(bounds.removeFromTop(44));
    bounds.removeFromTop(10);

    // Scan Button & Progress Bar
    auto scanRow = bounds.removeFromTop(34);
    startScanButton.setBounds(scanRow.removeFromLeft(280));
    scanRow.removeFromLeft(12);
    scanProgressBar.setBounds(scanRow);

    bounds.removeFromTop(6);
    scanStatusLabel.setBounds(bounds.removeFromTop(20));
    bounds.removeFromTop(6);

    // Manual Adjust Row
    auto manualRow = bounds.removeFromTop(28);
    manualLabel.setBounds(manualRow.removeFromLeft(190));
    lowNoteLabel.setBounds(manualRow.removeFromLeft(90));
    lowNoteCombo.setBounds(manualRow.removeFromLeft(75));
    manualRow.removeFromLeft(15);
    highNoteLabel.setBounds(manualRow.removeFromLeft(90));
    highNoteCombo.setBounds(manualRow.removeFromLeft(75));

    bounds.removeFromTop(10);

    // Vocal Class Banner
    vocalClassBanner.setBounds(bounds.removeFromTop(30));
    bounds.removeFromTop(12);

    // Recommend Section Header
    recommendSectionLabel.setBounds(bounds.removeFromTop(22));
    bounds.removeFromTop(6);

    // 5 Song Rows
    for (auto& row : songRows)
    {
        auto rowArea = bounds.removeFromTop(38);
        bounds.removeFromTop(4);

        if (row.titleLabel->isVisible())
        {
            row.singButton->setBounds(rowArea.removeFromRight(110).reduced(0, 3));
            row.badgeLabel->setBounds(rowArea.removeFromRight(130).reduced(0, 3));
            row.titleLabel->setBounds(rowArea.removeFromLeft(240));
            row.infoLabel->setBounds(rowArea);
        }
    }

    bounds.removeFromTop(8);

    // Bottom Action Buttons
    auto bottomRow = bounds.removeFromBottom(34);
    int halfW = (bottomRow.getWidth() - 12) / 2;
    viewAllInSongbookBtn.setBounds(bottomRow.removeFromLeft(halfW));
    saveAndCloseBtn.setBounds(bottomRow.removeFromRight(halfW));
}
