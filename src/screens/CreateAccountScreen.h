#pragma once
#include <JuceHeader.h>
#include "../auth/AuthManager.h"
#include "../pedalGUI/PedalComponent.h"

class CreateAccountScreen : public juce::Component
{
public:
    CreateAccountScreen(std::function<void()> goToLoginScreen, std::function<void()> goToHomeScreen)
        : onBackToLogin(goToLoginScreen), onAccountCreated(goToHomeScreen)
    {
        background = juce::ImageCache::getFromFile(getAppBundleResource("login-background.png"));



        // Labels on the left
        welcomeLabel.setText("Hi!", juce::dontSendNotification);
        welcomeLabel.setFont(juce::Font(36.0f, juce::Font::bold));
        welcomeLabel.setColour(juce::Label::textColourId, juce::Colours::white);
        addAndMakeVisible(welcomeLabel);

        subtext.setText("Please make an account to continue", juce::dontSendNotification);
        subtext.setFont(juce::Font(16.0f));
        subtext.setColour(juce::Label::textColourId, juce::Colours::white);
        addAndMakeVisible(subtext);

        // Inputs on the right
        emailEditor.setTextToShowWhenEmpty("Email Address", juce::Colours::black.withAlpha(0.6f));
        emailEditor.setColour(juce::TextEditor::backgroundColourId, juce::Colours::white.withAlpha(0.8f));
        emailEditor.setColour(juce::TextEditor::textColourId, juce::Colours::black);
        addAndMakeVisible(emailEditor);

        passwordEditor.setTextToShowWhenEmpty("Password", juce::Colours::black.withAlpha(0.6f));
        passwordEditor.setColour(juce::TextEditor::backgroundColourId, juce::Colours::white.withAlpha(0.8f));
        passwordEditor.setColour(juce::TextEditor::textColourId, juce::Colours::black);
        passwordEditor.setPasswordCharacter('*');
        addAndMakeVisible(passwordEditor);

        signUpButton.setButtonText("Create Account");
        signUpButton.onClick = [this] { handleSignUp(); };
        addAndMakeVisible(signUpButton);

        backButton.setButtonText("Back to Login");
        backButton.onClick = [this] { onBackToLogin(); };
        addAndMakeVisible(backButton);

        errorMessage.setText("", juce::dontSendNotification);
        errorMessage.setFont(juce::Font(16.0f));
        errorMessage.setColour(juce::Label::textColourId, juce::Colours::black);
        errorMessage.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(errorMessage);
    }

    void paint(juce::Graphics& g) override
    {
        if (background.isValid())
            g.drawImage(background, getLocalBounds().toFloat());
        else
            g.fillAll(juce::Colours::darkblue);
    }

    void resized() override
    {
        const int leftPadding = 60;
        const int topPadding = 120;
        const int fieldWidth = 260;
        const int fieldHeight = 40;
        const int spacing = 15;

        welcomeLabel.setBounds(leftPadding, topPadding - 70, 300, 50);
        subtext.setBounds(leftPadding, topPadding - 30, 400, 30);
        emailEditor.setBounds(getWidth() - leftPadding - fieldWidth, topPadding, fieldWidth, fieldHeight);
        passwordEditor.setBounds(emailEditor.getX(), emailEditor.getBottom() + spacing, fieldWidth, fieldHeight);
        errorMessage.setBounds(passwordEditor.getX(), passwordEditor.getBottom() + spacing, fieldWidth, 20);
        signUpButton.setBounds(errorMessage.getX(), errorMessage.getBottom() + spacing, fieldWidth, 40);
        backButton.setBounds(signUpButton.getX(), signUpButton.getBottom() + spacing, fieldWidth, 35);
    }


private:
    std::function<void()> onBackToLogin;
    std::function<void()> onAccountCreated;

    juce::Image background;

    juce::Label welcomeLabel;
    juce::Label subtext;
    juce::Label errorMessage;
    juce::TextEditor emailEditor;
    juce::TextEditor passwordEditor;
    juce::TextButton signUpButton;
    juce::TextButton backButton;

    void handleSignUp()
    {
        errorMessage.setText("", juce::dontSendNotification);
        
        juce::String emailText = emailEditor.getText();
        juce::String passwordText = passwordEditor.getText();
        
        if (emailText.isEmpty() || passwordText.isEmpty()) {
            errorMessage.setText("Please enter both email and password", juce::dontSendNotification);
            return;
        }
        
        auto& authManager = AuthManager::getInstance();
        AuthResponse response = authManager.signUp(emailText, passwordText);
        
        if (response.success) {
            onAccountCreated();
        } else {
            errorMessage.setText(response.errorMessage, juce::dontSendNotification);
        }
    }
};
