#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_core/juce_core.h>

class UpdateChecker
{
public:
    static constexpr const char* CURRENT_VERSION = "2.0.5";
    static constexpr const char* UPDATE_URL = "https://raw.githubusercontent.com/lachinhan/Hosi-Micro-Daw/main/version.json";
    static constexpr const char* FALLBACK_WEB_URL = "https://www.lachinhan.xyz";

    static void check(bool isManualCheck = false)
    {
        // Run check asynchronously in background thread to avoid blocking UI/Audio
        juce::Thread::launch([isManualCheck]() {
            bool success = false;
            juce::String latestVersion;
            juce::String releaseDate;
            juce::String downloadUrl = FALLBACK_WEB_URL;
            juce::String setupUrl = "https://github.com/lachinhan/Hosi-Micro-Daw/raw/main/LiveStream_Micro_DAW_Setup.exe";
            juce::String changelog;

            if (juce::String(UPDATE_URL).isEmpty())
            {
                juce::MessageManager::callAsync([isManualCheck]() {
                    if (isManualCheck)
                    {
                        juce::AlertWindow::showMessageBoxAsync(
                            juce::AlertWindow::InfoIcon,
                            juce::String::fromUTF8(u8"Kiểm Tra Cập Nhật"),
                            juce::String::fromUTF8(u8"✓ LiveStream Micro-DAW PRO v") + CURRENT_VERSION + juce::String::fromUTF8(u8" đang là phiên bản Studio mới nhất!"),
                            "OK"
                        );
                    }
                });
                return;
            }

            try
            {
                juce::URL url(UPDATE_URL);
                auto options = juce::URL::InputStreamOptions(juce::URL::ParameterHandling::inAddress)
                                    .withConnectionTimeoutMs(5000);
                
                std::unique_ptr<juce::InputStream> stream(url.createInputStream(options));
                if (stream != nullptr)
                {
                    juce::String jsonText = stream->readEntireStreamAsString();
                    auto jsonVar = juce::JSON::parse(jsonText);
                    if (jsonVar.isObject())
                    {
                        latestVersion = jsonVar.getProperty("latest_version", "").toString().trim();
                        releaseDate = jsonVar.getProperty("release_date", "").toString().trim();
                        downloadUrl = jsonVar.getProperty("download_url", FALLBACK_WEB_URL).toString().trim();
                        setupUrl = jsonVar.getProperty("setup_url", setupUrl).toString().trim();
                        changelog = jsonVar.getProperty("changelog", "").toString().trim();

                        if (latestVersion.isNotEmpty())
                        {
                            success = true;
                        }
                    }
                }
            }
            catch (...)
            {
                success = false;
            }

            // Return to Message Thread for UI interaction
            juce::MessageManager::callAsync([isManualCheck, success, latestVersion, releaseDate, downloadUrl, setupUrl, changelog]() {
                if (success && isNewerVersion(latestVersion, CURRENT_VERSION))
                {
                    juce::String message = juce::String::fromUTF8(u8"Đã có phiên bản mới: v") + latestVersion;
                    if (releaseDate.isNotEmpty())
                        message += " (" + releaseDate + ")";
                    message += "\n" + juce::String::fromUTF8(u8"Phiên bản hiện tại của bạn: v") + CURRENT_VERSION + "\n\n";

                    if (changelog.isNotEmpty())
                    {
                        message += juce::String::fromUTF8(u8"🌟 Chi tiết bản cập nhật:\n") + changelog + "\n\n";
                    }

                    message += juce::String::fromUTF8(u8"Bạn có muốn tải bản cập nhật mới ngay bây giờ không?");

                    juce::AlertWindow::showYesNoCancelBox(
                        juce::AlertWindow::InfoIcon,
                        juce::String::fromUTF8(u8"🎉 CẬP NHẬT LIVESTREAM MICRO-DAW"),
                        message,
                        juce::String::fromUTF8(u8"📥 Tải Bản Cài Đặt (Setup)"),
                        juce::String::fromUTF8(u8"🌐 Mở Trang Web"),
                        juce::String::fromUTF8(u8"Để Sau"),
                        nullptr,
                        juce::ModalCallbackFunction::create([downloadUrl, setupUrl](int result) {
                            if (result == 1) // Tải Setup
                            {
                                juce::URL(setupUrl).launchInDefaultBrowser();
                            }
                            else if (result == 2) // Mở Website
                            {
                                juce::URL(downloadUrl).launchInDefaultBrowser();
                            }
                        })
                    );
                }
                else if (isManualCheck)
                {
                    if (success)
                    {
                        juce::AlertWindow::showMessageBoxAsync(
                            juce::AlertWindow::InfoIcon,
                            juce::String::fromUTF8(u8"Kiểm Tra Cập Nhật"),
                            juce::String::fromUTF8(u8"✓ Bạn đang sử dụng phiên bản mới nhất (v") + CURRENT_VERSION + juce::String::fromUTF8(u8")!\nCảm ơn bạn đã luôn đồng hành cùng LiveStream Micro-DAW."),
                            "OK"
                        );
                    }
                    else
                    {
                        juce::AlertWindow::showMessageBoxAsync(
                            juce::AlertWindow::WarningIcon,
                            juce::String::fromUTF8(u8"Kiểm Tra Cập Nhật"),
                            juce::String::fromUTF8(u8"Không thể kết nối đến máy chủ kiểm tra cập nhật lúc này.\nVui lòng kiểm tra kết nối mạng hoặc truy cập website: ") + FALLBACK_WEB_URL,
                            "OK"
                        );
                    }
                }
            });
        });
    }

    static bool isNewerVersion(const juce::String& remote, const juce::String& local)
    {
        auto parseVer = [](const juce::String& v) -> std::vector<int> {
            std::vector<int> parts;
            juce::StringArray tokens;
            tokens.addTokens(v.trimCharactersAtStart("vV"), ".", "");
            for (const auto& t : tokens)
                parts.push_back(t.getIntValue());
            while (parts.size() < 3)
                parts.push_back(0);
            return parts;
        };

        auto r = parseVer(remote);
        auto l = parseVer(local);

        for (size_t i = 0; i < 3; ++i)
        {
            if (r[i] > l[i]) return true;
            if (r[i] < l[i]) return false;
        }
        return false;
    }
};
