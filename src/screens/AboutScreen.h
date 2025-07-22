#pragma once
#include <JuceHeader.h>
#include "../pedalGUI/PedalComponent.h"

class AboutScreen : public juce::Component {
public:
    AboutScreen(std::function<void()> onBack)
        : goBack(std::move(onBack)) {
        
        // Load the background image
        backgroundImage = juce::ImageCache::getFromFile(
            getAppBundleResource("about.png"));

        // Back button setup
        backButton.setButtonText("Back");
        backButton.onClick = [this] { goBack(); };
        addAndMakeVisible(backButton);

        // Grey background component
        addAndMakeVisible(textBackground);
        textBackground.setInterceptsMouseClicks(false, false);

        // About text
        aboutText.setText(
        "This guitar pedal board application is built from the ground up in C++ using the professional JUCE framework, delivering real-time audio processing with studio-quality effects and modern software architecture.\n\n"
        
        "Features eight guitar pedals: Overdrive, Distortion, Reverb, Chorus, Compressor, Tremolo, Octave, and Blues Driver. Each uses advanced DSP algorithms for unique, responsive tones.\n\n"
        
        "Includes real-time audio streaming, drag-and-drop pedal arrangement, save/load functionality, and cloud authentication. Create and save custom pedal board configurations.\n\n"
    
        
        "Feel free to reach out to me (max8alton@gmail.com) with any questions, suggestions, or comments. This entire project is on my public GitHub repo https://github.com/maxxcraig/Max-s-Guitar-Pedal-App.",

            juce::dontSendNotification);
        aboutText.setJustificationType(juce::Justification::centred);
        aboutText.setColour(juce::Label::textColourId, juce::Colours::white);
        aboutText.setFont(juce::Font(15.0f));
        addAndMakeVisible(aboutText);

        // Load the max.png image
        maxImage = juce::ImageCache::getFromFile(
            getAppBundleResource("max.png"));

        // "Me!" label
        meLabel.setText("Me!", juce::dontSendNotification);
        meLabel.setJustificationType(juce::Justification::centred);
        meLabel.setColour(juce::Label::textColourId, juce::Colours::white);
        meLabel.setFont(juce::Font(20.0f, juce::Font::bold));
        addAndMakeVisible(meLabel);
    }

    void paint(juce::Graphics& g) override {
        if (backgroundImage.isValid())
            g.drawImage(backgroundImage, getLocalBounds().toFloat());
        else
            g.fillAll(juce::Colours::darkblue);

        // Draw the max.png image on the right side with rounded corners
        if (maxImage.isValid()) {
            int imageSize = 500; // Size for the image (2.5x larger)
            int imageX = getWidth() - 550; // Position from right
            int imageY = getHeight() / 2 - imageSize / 2; // Center vertically
            
            juce::Rectangle<float> imageArea(imageX, imageY, imageSize, imageSize);
            
            // Save the current graphics state
            juce::Graphics::ScopedSaveState saveState(g);
            
            // Create a rounded rectangle path for clipping
            juce::Path roundedRect;
            roundedRect.addRoundedRectangle(imageArea, 20.0f);
            g.reduceClipRegion(roundedRect);
            
            // Draw the image with rounded corners
            g.drawImage(maxImage, imageArea, juce::RectanglePlacement::centred);
        }
    }

    void resized() override {
        backButton.setBounds(20, 20, 100, 30);

        int width = getWidth();
        int height = getHeight();

        // Left side text area (centered vertically)
        int marginLeft = 80;
        int textWidth = width - 700; // Leave more space for larger image
        int textHeight = 450;
        int marginTop = (height - textHeight) / 2; // Center vertically
        
        juce::Rectangle<int> textArea(marginLeft, marginTop, textWidth, textHeight);
        textBackground.setBounds(textArea.expanded(20, 20));
        aboutText.setBounds(textArea);

        // "Me!" label closer to the bottom of the larger image
        int imageSize = 500;
        int imageX = getWidth() - 550;
        int imageY = getHeight() / 2 - imageSize / 2;
        meLabel.setBounds(imageX, imageY + imageSize - 35, imageSize, 40); // Moved closer to image
    }

private:
    juce::TextButton backButton;
    juce::Label aboutText;
    juce::Label meLabel;
    juce::Image backgroundImage;
    juce::Image maxImage;
    std::function<void()> goBack;

    // Background for the text
    class TextBackgroundComponent : public juce::Component {
        void paint(juce::Graphics& g) override {
            g.setColour(juce::Colours::darkgrey.withAlpha(0.8f));
            g.fillRoundedRectangle(getLocalBounds().toFloat(), 20.0f);
        }
    } textBackground;
};
