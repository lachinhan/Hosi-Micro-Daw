#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <BinaryData.h>

class DonateOverlay : public juce::Component
{
public:
    DonateOverlay();
    ~DonateOverlay() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;

    std::function<void()> onCloseClicked;

private:
    enum class QrType
    {
        VietQrMb,
        MoMo,
        PayPal,
        MbCard
    };

    QrType currentQr{ QrType::VietQrMb };

    juce::Label titleLabel;
    juce::Label subtitleLabel;
    juce::TextButton closeButton{ juce::String::fromUTF8(u8"✕ ĐÓNG") };

    // QR Type Switcher Buttons
    juce::TextButton btnVietQr{ juce::String::fromUTF8(u8"🏦 MBBANK") };
    juce::TextButton btnMoMo{ juce::String::fromUTF8(u8"🟣 MOMO") };
    juce::TextButton btnPayPal{ juce::String::fromUTF8(u8"🅿️ PAYPAL") };
    juce::TextButton btnMbCard{ juce::String::fromUTF8(u8"💳 MB CARD") };

    // Bank Account Info Labels
    juce::Label accountHolderLabel;
    juce::Label accountNumberLabel;
    juce::Label bankNameLabel;
    juce::Label amountLabel;
    juce::Label contentLabel;

    // Quick Copy Action Buttons
    juce::TextButton copyAccNumberButton{ juce::String::fromUTF8(u8"📋 SAO CHÉP STK") };
    juce::TextButton copyContentButton{ juce::String::fromUTF8(u8"📋 SAO CHÉP NỘI DUNG") };
    juce::Label copyToastLabel;

    // Support Hotline & Service Box
    juce::Label supportInfoLabel;

    juce::Image imgVietQr;
    juce::Image imgMoMo;
    juce::Image imgPayPal;
    juce::Image imgMbCard;

    void setQrType(QrType type);
    void updateButtonsUI();
    void showToast(const juce::String& message);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DonateOverlay)
};
