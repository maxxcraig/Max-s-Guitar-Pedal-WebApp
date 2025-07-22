#pragma once
#include <JuceHeader.h>
#include "../pedalGUI/PedalComponent.h"

class HomeScreen : public juce::Component
{
public:
    HomeScreen(std::function<void()> goToPedalBoard,
               std::function<void()> goToAbout,
               std::function<void()> logout,
               std::function<void()> goToSavedBoards)
        : onCreatePedalBoard(goToPedalBoard),
          onAbout(goToAbout),
          onLogout(logout),
          onViewBoards(goToSavedBoards) 
    {
        // Load background image
        backgroundImage = juce::ImageCache::getFromFile(getAppBundleResource("home-background.png"));

        // Intro paragraph
        introLabel.setText(
            "Welcome to my Guitar Pedal App!\n"
            "Create, experiment with, and save custom pedal boards.\n"
            "Perfect for designing your tone and trying out new effects.\n"
            "Just plug in with your audio interface of choice, allow microphone access, and play!\n",
            juce::dontSendNotification);
        introLabel.setColour(juce::Label::textColourId, juce::Colours::white);
        introLabel.setFont(juce::Font(16.0f)); // Match button font
        introLabel.setJustificationType(juce::Justification::topLeft);
        introLabel.setInterceptsMouseClicks(false, false);
        addAndMakeVisible(introLabel);

        // Top nav buttons
        aboutButton.setButtonText("About");
        aboutButton.onClick = onAbout;
        addAndMakeVisible(aboutButton);

        viewBoardsButton.setButtonText("View Saved Boards");
        viewBoardsButton.onClick = [this]() {
            if (onViewBoards) onViewBoards();
        };
        addAndMakeVisible(viewBoardsButton);

        logoutButton.setButtonText("Log Out");
        logoutButton.onClick = onLogout;
        addAndMakeVisible(logoutButton);

        // Create pedal board button
        createPedalBoardButton.setButtonText("Create New Pedal Board");
        createPedalBoardButton.setColour(juce::TextButton::buttonColourId, juce::Colours::grey);
        createPedalBoardButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
        createPedalBoardButton.onClick = onCreatePedalBoard;
        addAndMakeVisible(createPedalBoardButton);
    }

    void paint(juce::Graphics& g) override
    {
        if (backgroundImage.isValid())
            g.drawImage(backgroundImage, getLocalBounds().toFloat());
        else
            g.fillAll(juce::Colours::darkcyan);
    }

    void resized() override
    {
        auto area = getLocalBounds();

        // Top nav buttons
        auto topBar = area.removeFromTop(50).reduced(10);
        const int btnWidth = 150;
        const int spacing = 10;

        logoutButton.setBounds(topBar.removeFromRight(btnWidth));
        topBar.removeFromRight(spacing);
        viewBoardsButton.setBounds(topBar.removeFromRight(btnWidth));
        topBar.removeFromRight(spacing);
        aboutButton.setBounds(topBar.removeFromRight(btnWidth));

        // Intro and Create button area - moved down to center better
        int marginLeft = 80;
        int marginTop = 220;  // Moved down from 150 to 220
        int contentWidth = 500;

        introLabel.setBounds(marginLeft, marginTop, contentWidth, 140);
        createPedalBoardButton.setBounds(marginLeft, marginTop + 120, 300, 50);
    }

private:
    juce::Image backgroundImage;

    juce::Label introLabel;
    juce::TextButton aboutButton;
    juce::TextButton viewBoardsButton;
    juce::TextButton logoutButton;
    juce::TextButton createPedalBoardButton;

    std::function<void()> onCreatePedalBoard;
    std::function<void()> onAbout;
    std::function<void()> onLogout;
    std::function<void()> onViewBoards; 
};
