#include "DonateOverlay.h"

DonateOverlay::DonateOverlay()
{
    // Load Images from BinaryData
    imgVietQr = juce::ImageCache::getFromMemory(BinaryData::qr_vietqr_png, BinaryData::qr_vietqr_pngSize);
    imgMoMo = juce::ImageCache::getFromMemory(BinaryData::qr_momo_jpg, BinaryData::qr_momo_jpgSize);
    imgPayPal = juce::ImageCache::getFromMemory(BinaryData::qr_paypal_jpg, BinaryData::qr_paypal_jpgSize);
    imgMbCard = juce::ImageCache::getFromMemory(BinaryData::qr_mbbank_jpg, BinaryData::qr_mbbank_jpgSize);

    // Title & Subtitle
    titleLabel.setText(juce::String::fromUTF8(u8"💖 ỦNG HỘ PHÁT TRIỂN / DONATE LIVESTREAM MICRO-DAW"), juce::dontSendNotification);
    titleLabel.setFont(juce::FontOptions(17.0f, juce::Font::bold));
    titleLabel.setColour(juce::Label::textColourId, juce::Colour(0xfff59e0b)); // Amber Gold
    titleLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(titleLabel);

    subtitleLabel.setText(juce::String::fromUTF8(u8"Sự ủng hộ của bạn là nguồn động lực to lớn giúp tác giả duy trì, nâng cấp và phát triển ứng dụng miễn phí cho cộng đồng!"), juce::dontSendNotification);
    subtitleLabel.setFont(juce::FontOptions(11.5f, juce::Font::plain));
    subtitleLabel.setColour(juce::Label::textColourId, juce::Colour(0xff94a3b8));
    subtitleLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(subtitleLabel);

    // Close Button
    closeButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff2a2e39));
    closeButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    closeButton.onClick = [this]() {
        if (onCloseClicked)
            onCloseClicked();
    };
    addAndMakeVisible(closeButton);

    // QR Type Selectors
    btnVietQr.onClick = [this] { setQrType(QrType::VietQrMb); };
    btnMoMo.onClick = [this] { setQrType(QrType::MoMo); };
    btnPayPal.onClick = [this] { setQrType(QrType::PayPal); };
    btnMbCard.onClick = [this] { setQrType(QrType::MbCard); };
    addAndMakeVisible(btnVietQr);
    addAndMakeVisible(btnMoMo);
    addAndMakeVisible(btnPayPal);
    addAndMakeVisible(btnMbCard);

    // Bank Account Info Labels
    accountHolderLabel.setFont(juce::FontOptions(14.0f, juce::Font::bold));
    accountHolderLabel.setColour(juce::Label::textColourId, juce::Colour(0xff38bdf8)); // Sky Blue
    addAndMakeVisible(accountHolderLabel);

    accountNumberLabel.setFont(juce::FontOptions(16.0f, juce::Font::bold));
    accountNumberLabel.setColour(juce::Label::textColourId, juce::Colour(0xfffcd34d)); // Gold
    addAndMakeVisible(accountNumberLabel);

    bankNameLabel.setFont(juce::FontOptions(13.0f, juce::Font::plain));
    bankNameLabel.setColour(juce::Label::textColourId, juce::Colour(0xffcbd5e1));
    addAndMakeVisible(bankNameLabel);

    amountLabel.setFont(juce::FontOptions(13.0f, juce::Font::bold));
    amountLabel.setColour(juce::Label::textColourId, juce::Colour(0xff10b981)); // Emerald Green
    addAndMakeVisible(amountLabel);

    contentLabel.setFont(juce::FontOptions(13.0f, juce::Font::plain));
    contentLabel.setColour(juce::Label::textColourId, juce::Colour(0xffe2e8f0));
    addAndMakeVisible(contentLabel);

    // Quick Copy Action Buttons
    copyAccNumberButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff0369a1));
    copyAccNumberButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    copyAccNumberButton.setTooltip(juce::String::fromUTF8(u8"Sao chép thông tin tài khoản"));
    copyAccNumberButton.onClick = [this] {
        if (currentQr == QrType::PayPal)
        {
            juce::SystemClipboard::copyTextToClipboard("Nhan La Chi");
            showToast(juce::String::fromUTF8(u8"✓ Đã sao chép tên PayPal: Nhan La Chi"));
        }
        else
        {
            juce::SystemClipboard::copyTextToClipboard("0908107000");
            showToast(juce::String::fromUTF8(u8"✓ Đã sao chép STK/SĐT: 0908107000"));
        }
    };
    addAndMakeVisible(copyAccNumberButton);

    copyContentButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff1e293b));
    copyContentButton.setColour(juce::TextButton::textColourOffId, juce::Colour(0xffcbd5e1));
    copyContentButton.setTooltip(juce::String::fromUTF8(u8"Sao chép nội dung: Ung ho LiveStream Micro-DAW"));
    copyContentButton.onClick = [this] {
        juce::SystemClipboard::copyTextToClipboard("Ung ho LiveStream Micro-DAW");
        showToast(juce::String::fromUTF8(u8"✓ Đã sao chép nội dung chuyển khoản!"));
    };
    addAndMakeVisible(copyContentButton);

    copyToastLabel.setFont(juce::FontOptions(12.0f, juce::Font::bold));
    copyToastLabel.setColour(juce::Label::textColourId, juce::Colour(0xff34d399));
    copyToastLabel.setJustificationType(juce::Justification::centred);
    addChildComponent(copyToastLabel);

    // Support Hotline & Service Box
    supportInfoLabel.setText(
        juce::String::fromUTF8(u8"🛠️ HỖ TRỢ & DỊCH VỤ SETUP ÂM THANH LIVESTREAM / HÁT LIVE CHUYÊN NGHIỆP:\n")
        + juce::String::fromUTF8(u8"• Cài đặt trọn bộ Plugin VST3 hay nhất (Auto-Tune, FabFilter, Reverb, Delay) qua UltraViewer / TeamViewer.\n")
        + juce::String::fromUTF8(u8"• Tinh chỉnh âm thanh chuẩn phòng thu theo từng chất giọng và micro của bạn.\n")
        + juce::String::fromUTF8(u8"👉 Liên hệ Hotline / Zalo: 0908.107.000 (La Chí Nhân)"),
        juce::dontSendNotification
    );
    supportInfoLabel.setFont(juce::FontOptions(11.0f, juce::Font::plain));
    supportInfoLabel.setColour(juce::Label::textColourId, juce::Colour(0xff94a3b8));
    supportInfoLabel.setJustificationType(juce::Justification::topLeft);
    addAndMakeVisible(supportInfoLabel);

    setQrType(QrType::VietQrMb);
}

void DonateOverlay::setQrType(QrType type)
{
    currentQr = type;
    updateButtonsUI();

    if (currentQr == QrType::VietQrMb || currentQr == QrType::MbCard)
    {
        accountHolderLabel.setText(juce::String::fromUTF8(u8"👤 Chủ tài khoản: LA CHI NHAN"), juce::dontSendNotification);
        accountNumberLabel.setText(juce::String::fromUTF8(u8"💳 Số tài khoản: 0908107000"), juce::dontSendNotification);
        bankNameLabel.setText(juce::String::fromUTF8(u8"🏦 Ngân hàng: TMCP Quân đội (MBBank)"), juce::dontSendNotification);
        amountLabel.setText(juce::String::fromUTF8(u8"💵 Số tiền: 150,000 VNĐ (hoặc tùy tâm)"), juce::dontSendNotification);
        contentLabel.setText(juce::String::fromUTF8(u8"📝 Nội dung: Ung ho LiveStream Micro-DAW"), juce::dontSendNotification);
        copyAccNumberButton.setButtonText(juce::String::fromUTF8(u8"📋 SAO CHÉP STK"));
    }
    else if (currentQr == QrType::MoMo)
    {
        accountHolderLabel.setText(juce::String::fromUTF8(u8"👤 Chủ tài khoản: LA CHI NHAN"), juce::dontSendNotification);
        accountNumberLabel.setText(juce::String::fromUTF8(u8"🟣 Ví MoMo: 0908107000"), juce::dontSendNotification);
        bankNameLabel.setText(juce::String::fromUTF8(u8"📱 Ứng dụng: Ví điện tử MoMo"), juce::dontSendNotification);
        amountLabel.setText(juce::String::fromUTF8(u8"💵 Số tiền: 150.000đ (hoặc tùy tâm)"), juce::dontSendNotification);
        contentLabel.setText(juce::String::fromUTF8(u8"📝 Nội dung: Ung ho LiveStream Micro-DAW"), juce::dontSendNotification);
        copyAccNumberButton.setButtonText(juce::String::fromUTF8(u8"📋 SAO CHÉP SĐT MOMO"));
    }
    else // PayPal
    {
        accountHolderLabel.setText(juce::String::fromUTF8(u8"👤 Người nhận (Recipient): Nhan La Chi"), juce::dontSendNotification);
        accountNumberLabel.setText(juce::String::fromUTF8(u8"🅿️ PayPal: Nhan La Chi"), juce::dontSendNotification);
        bankNameLabel.setText(juce::String::fromUTF8(u8"🌐 Thanh toán quốc tế: PayPal (USD / EUR / VNĐ)"), juce::dontSendNotification);
        amountLabel.setText(juce::String::fromUTF8(u8"💵 Số tiền (Amount): Tùy tâm (Ví dụ: $5 / $10 / 150,000đ)"), juce::dontSendNotification);
        contentLabel.setText(juce::String::fromUTF8(u8"📝 Ghi chú (Note): Ung ho LiveStream Micro-DAW"), juce::dontSendNotification);
        copyAccNumberButton.setButtonText(juce::String::fromUTF8(u8"📋 SAO CHÉP TÊN PAYPAL"));
    }

    repaint();
}

void DonateOverlay::updateButtonsUI()
{
    auto setBtnStyle = [](juce::TextButton& btn, bool active, juce::Colour activeCol) {
        if (active)
        {
            btn.setColour(juce::TextButton::buttonColourId, activeCol);
            btn.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
        }
        else
        {
            btn.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff1e293b));
            btn.setColour(juce::TextButton::textColourOffId, juce::Colour(0xff94a3b8));
        }
    };

    setBtnStyle(btnVietQr, currentQr == QrType::VietQrMb, juce::Colour(0xff0284c7)); // Sky Blue
    setBtnStyle(btnMoMo, currentQr == QrType::MoMo, juce::Colour(0xffa855f7)); // Purple / Pink
    setBtnStyle(btnPayPal, currentQr == QrType::PayPal, juce::Colour(0xff0070ba)); // PayPal Classic Blue
    setBtnStyle(btnMbCard, currentQr == QrType::MbCard, juce::Colour(0xff059669)); // Emerald Green
}

void DonateOverlay::showToast(const juce::String& message)
{
    copyToastLabel.setText(message, juce::dontSendNotification);
    copyToastLabel.setVisible(true);

    // Auto hide after 2.5 seconds
    juce::Timer::callAfterDelay(2500, [safe = juce::Component::SafePointer<DonateOverlay>(this)]() {
        if (safe != nullptr)
        {
            safe->copyToastLabel.setVisible(false);
        }
    });
}

void DonateOverlay::paint(juce::Graphics& g)
{
    // Dark Backdrop
    g.fillAll(juce::Colour(0xd8090c14));

    // Center Card Bounds
    auto bounds = getLocalBounds().reduced(24);
    g.setColour(juce::Colour(0xff141824));
    g.fillRoundedRectangle(bounds.toFloat(), 12.0f);

    g.setColour(juce::Colour(0xff2a3449));
    g.drawRoundedRectangle(bounds.toFloat(), 12.0f, 1.5f);

    // Draw QR Code in designated left area
    auto contentArea = bounds.reduced(16);
    contentArea.removeFromTop(68); // Below title & subtitle
    contentArea.removeFromBottom(72); // Above footer

    const int qrBoxW = std::min(300, contentArea.getWidth() / 2 - 10);
    auto qrRect = contentArea.removeFromLeft(qrBoxW).reduced(6);

    // Background container for QR
    g.setColour(juce::Colour(0xff0d101a));
    g.fillRoundedRectangle(qrRect.toFloat(), 8.0f);
    g.setColour(juce::Colour(0xff1e293b));
    g.drawRoundedRectangle(qrRect.toFloat(), 8.0f, 1.0f);

    juce::Image* targetImg = nullptr;
    if (currentQr == QrType::VietQrMb && imgVietQr.isValid())
        targetImg = &imgVietQr;
    else if (currentQr == QrType::MoMo && imgMoMo.isValid())
        targetImg = &imgMoMo;
    else if (currentQr == QrType::PayPal && imgPayPal.isValid())
        targetImg = &imgPayPal;
    else if (currentQr == QrType::MbCard && imgMbCard.isValid())
        targetImg = &imgMbCard;

    if (targetImg != nullptr && targetImg->isValid())
    {
        g.drawImageWithin(*targetImg,
                          qrRect.getX() + 6, qrRect.getY() + 6,
                          qrRect.getWidth() - 12, qrRect.getHeight() - 12,
                          juce::RectanglePlacement::centred | juce::RectanglePlacement::onlyReduceInSize,
                          false);
    }
}

void DonateOverlay::resized()
{
    auto bounds = getLocalBounds().reduced(24);

    // Header Area
    auto headerArea = bounds.removeFromTop(38);
    closeButton.setBounds(headerArea.removeFromRight(80).reduced(2, 4));
    titleLabel.setBounds(headerArea);

    subtitleLabel.setBounds(bounds.removeFromTop(20));

    bounds.removeFromTop(8);

    // Footer Support Box
    auto footerArea = bounds.removeFromBottom(70);
    supportInfoLabel.setBounds(footerArea.reduced(6, 4));

    // Content Area
    auto contentArea = bounds.reduced(6);
    
    // Left side: QR Image Area (calculated in paint)
    const int qrBoxW = std::min(300, contentArea.getWidth() / 2 - 10);
    contentArea.removeFromLeft(qrBoxW);
    contentArea.removeFromLeft(16); // Gap

    // Right side: Controls & Account Details
    auto rightArea = contentArea;

    // QR Switcher Buttons Row (4 Buttons)
    auto switchRow = rightArea.removeFromTop(32);
    const int btnW = (switchRow.getWidth() - 12) / 4;
    btnVietQr.setBounds(switchRow.removeFromLeft(btnW).reduced(2, 2));
    btnMoMo.setBounds(switchRow.removeFromLeft(btnW).reduced(2, 2));
    btnPayPal.setBounds(switchRow.removeFromLeft(btnW).reduced(2, 2));
    btnMbCard.setBounds(switchRow.reduced(2, 2));

    rightArea.removeFromTop(12);

    accountHolderLabel.setBounds(rightArea.removeFromTop(24));
    accountNumberLabel.setBounds(rightArea.removeFromTop(28));
    bankNameLabel.setBounds(rightArea.removeFromTop(22));
    amountLabel.setBounds(rightArea.removeFromTop(24));
    contentLabel.setBounds(rightArea.removeFromTop(24));

    rightArea.removeFromTop(8);

    auto copyRow = rightArea.removeFromTop(30);
    copyAccNumberButton.setBounds(copyRow.removeFromLeft(160).reduced(2, 1));
    copyContentButton.setBounds(copyRow.removeFromLeft(190).reduced(2, 1));

    rightArea.removeFromTop(4);
    copyToastLabel.setBounds(rightArea.removeFromTop(22));
}
