#pragma once

#include <JuceHeader.h>
#include "../auth/AuthManager.h"
#include "../pedalGUI/PedalComponent.h"

class LoginScreen : public juce::Component
{
public:
    LoginScreen(std::function<void()> goToHomeScreen, std::function<void()> goToCreateAccountScreen)
        : onLoginSuccess(goToHomeScreen), onCreateAccount(goToCreateAccountScreen)
    {
        // Set background image
        backgroundImage = juce::ImageCache::getFromFile(getAppBundleResource("login-background.png"));

        addAndMakeVisible(title);
        title.setText("Welcome Back", juce::dontSendNotification);
        title.setFont(juce::Font(32.0f, juce::Font::bold));
        title.setColour(juce::Label::textColourId, juce::Colours::white);
        title.setJustificationType(juce::Justification::centredLeft);

        addAndMakeVisible(subtext);
        subtext.setText("Please sign in to access", juce::dontSendNotification);
        subtext.setFont(juce::Font(16.0f));
        subtext.setColour(juce::Label::textColourId, juce::Colours::white);
        subtext.setJustificationType(juce::Justification::centredLeft);

        setupTextEditor(email, "Email Address");
        setupTextEditor(password, "Password", true);

        addAndMakeVisible(loginButton);
        loginButton.setButtonText("Sign in now");
        loginButton.setColour(juce::TextButton::buttonColourId, juce::Colours::orange);
        loginButton.onClick = [this] { handleLogin(); };

        addAndMakeVisible(errorMessage);
        errorMessage.setText("", juce::dontSendNotification);
        errorMessage.setFont(juce::Font(16.0f));
        errorMessage.setColour(juce::Label::textColourId, juce::Colours::black);
        errorMessage.setJustificationType(juce::Justification::centred);

        addAndMakeVisible(createAccountButton);
        createAccountButton.setButtonText("Create Account");
        createAccountButton.setColour(juce::TextButton::buttonColourId, juce::Colours::lightblue);
        createAccountButton.onClick = [this] { onCreateAccount(); };

        setSize(800, 500);
    }

    void paint(juce::Graphics& g) override
    {
        if (backgroundImage.isValid())
            g.drawImage(backgroundImage, getLocalBounds().toFloat());
        else
            g.fillAll(juce::Colours::darkgrey);

        g.setColour(juce::Colours::white.withAlpha(0.85f));
    }

    void resized() override
    {
        const int leftPadding = 60;
        const int topPadding = 120;
        const int fieldWidth = 260;
        const int fieldHeight = 40;
        const int spacing = 15;

        title.setBounds(leftPadding, topPadding - 70, 300, 50);
        subtext.setBounds(leftPadding, topPadding - 30, 400, 30);
        email.setBounds(getWidth() - leftPadding - fieldWidth, topPadding, fieldWidth, fieldHeight);
        password.setBounds(email.getX(), email.getBottom() + spacing, fieldWidth, fieldHeight);
        errorMessage.setBounds(password.getX(), password.getBottom() + spacing, fieldWidth, 20);
        loginButton.setBounds(errorMessage.getX(), errorMessage.getBottom() + spacing, fieldWidth, 40);
        createAccountButton.setBounds(loginButton.getX(), loginButton.getBottom() + spacing, fieldWidth, 35);
    }
    
    // Authentication getter methods
    juce::String getEmail() const { return email.getText(); }
    juce::String getPassword() const { return password.getText(); }

private:
    std::function<void()> onLoginSuccess;
    std::function<void()> onCreateAccount;

    juce::Label title;
    juce::Label subtext;
    juce::Label errorMessage;

    juce::TextEditor email;
    juce::TextEditor password;

    juce::TextButton loginButton;
    juce::TextButton createAccountButton;

    juce::Image backgroundImage;

    void setupTextEditor(juce::TextEditor& editor, const juce::String& placeholder, bool isPassword = false)
    {
        addAndMakeVisible(editor);
        editor.setTextToShowWhenEmpty(placeholder, juce::Colours::black.withAlpha(0.6f));
        editor.setColour(juce::TextEditor::backgroundColourId, juce::Colours::white.withAlpha(0.8f));
        editor.setColour(juce::TextEditor::textColourId, juce::Colours::black);
        editor.setColour(juce::TextEditor::outlineColourId, juce::Colours::transparentBlack);
        editor.setFont(juce::Font(16.0f));
        if (isPassword)
            editor.setPasswordCharacter('*');
    }

    void handleLogin()
    {
        errorMessage.setText("", juce::dontSendNotification);
        
        juce::String emailText = email.getText();
        juce::String passwordText = password.getText();
        
        if (emailText.isEmpty() || passwordText.isEmpty()) {
            errorMessage.setText("Please enter both email and password", juce::dontSendNotification);
            return;
        }
        
        auto& authManager = AuthManager::getInstance();
        AuthResponse response = authManager.signIn(emailText, passwordText);
        
        if (response.success) {
            onLoginSuccess();
        } else {
            errorMessage.setText(response.errorMessage, juce::dontSendNotification);
        }
    }
};
