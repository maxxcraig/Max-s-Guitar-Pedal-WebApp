#include <JuceHeader.h>
#include "ScreenRouter.h"

class MainApp : public juce::JUCEApplication
{
public:
    const juce::String getApplicationName() override       { return "GuitarPedalApp"; }
    const juce::String getApplicationVersion() override    { return "0.1.0"; }
    bool moreThanOneInstanceAllowed() override             { return true; }

    void initialise(const juce::String&) override
    {
        mainWindow.reset(new juce::DocumentWindow(
            getApplicationName(),
            juce::Colours::blue,
            juce::DocumentWindow::allButtons
        ));

        mainWindow->setUsingNativeTitleBar(true);
        mainWindow->setContentOwned(new ScreenRouter(), true);
        mainWindow->setResizable(true, false);
        mainWindow->centreWithSize(1000, 600);
        mainWindow->setVisible(true);
    }

    void shutdown() override
    {
        mainWindow = nullptr;
    }

    void systemRequestedQuit() override
    {
        quit();
    }

private:
    std::unique_ptr<juce::DocumentWindow> mainWindow;
};

START_JUCE_APPLICATION(MainApp)
