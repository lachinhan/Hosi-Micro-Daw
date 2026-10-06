#include <juce_gui_basics/juce_gui_basics.h>
#include "MainComponent.h"
#include <BinaryData.h>

class LiveStreamMicroDAWApplication : public juce::JUCEApplication
{
public:
    LiveStreamMicroDAWApplication() = default;

    const juce::String getApplicationName() override { return "LiveStream Micro-DAW"; }
    const juce::String getApplicationVersion() override { return "1.0.0"; }
    bool moreThanOneInstanceAllowed() override { return false; }

    void initialise(const juce::String& /*commandLine*/) override
    {
        mainWindow = std::make_unique<MainWindow>(getApplicationName());
    }

    void shutdown() override
    {
        mainWindow.reset();
    }

    void systemRequestedQuit() override
    {
        quit();
    }

    void anotherInstanceStarted(const juce::String& /*commandLine*/) override
    {
    }

    class MainWindow : public juce::DocumentWindow
    {
    public:
        explicit MainWindow(juce::String name)
            : DocumentWindow(
                name,
                juce::Colour(0xff0b0d13),
                DocumentWindow::allButtons
            )
        {
            auto iconImage = juce::ImageFileFormat::loadFrom(BinaryData::icon_png, BinaryData::icon_pngSize);
            if (iconImage.isValid())
            {
                setIcon(iconImage);
            }

            setUsingNativeTitleBar(true);
            setContentOwned(new MainComponent(), true);

#if JUCE_IOS || JUCE_ANDROID
            setFullScreen(true);
#else
            setResizable(true, true);
            setResizeLimits(700, 500, 1600, 1200);
            centreWithSize(getWidth(), getHeight());
#endif
            setVisible(true);
        }

        void closeButtonPressed() override
        {
            juce::JUCEApplication::getInstance()->systemRequestedQuit();
        }

    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainWindow)
    };

private:
    std::unique_ptr<MainWindow> mainWindow;
};

START_JUCE_APPLICATION(LiveStreamMicroDAWApplication)
