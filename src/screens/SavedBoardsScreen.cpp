#include "SavedBoardsScreen.h"
#include "../pedalGUI/PedalComponent.h"

SavedBoardsScreen::SavedBoardsScreen(std::function<void()> goHome,
                                     std::function<void(const juce::String&)> loadBoard)
    : onBackToHome(goHome), onLoadBoard(loadBoard)

{
    backgroundImage = juce::ImageCache::getFromFile(getAppBundleResource("home-background.png"));

    backButton.setButtonText("Home");
    backButton.setColour(juce::TextButton::buttonColourId, juce::Colours::grey);
    backButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    backButton.onClick = [this]() { onBackToHome(); };
    addAndMakeVisible(backButton);

    // Check if user is authenticated
    auto& authManager = AuthManager::getInstance();
    if (!authManager.isLoggedIn()) {
        noBoardsLabel.setText("Please log in to view saved boards.", juce::dontSendNotification);
        noBoardsLabel.setColour(juce::Label::textColourId, juce::Colours::white);
        noBoardsLabel.setFont(juce::Font(18.0f));
        noBoardsLabel.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(noBoardsLabel);
        return;
    }

    // Show loading message
    noBoardsLabel.setText("Loading boards...", juce::dontSendNotification);
    noBoardsLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    noBoardsLabel.setFont(juce::Font(18.0f));
    noBoardsLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(noBoardsLabel);

    // Load saved boards from cloud
    loadBoards();

    // Remove loading message
    removeChildComponent(&noBoardsLabel);

    // If no saved boards
    if (savedBoards.isEmpty()) {
        noBoardsLabel.setText("No saved boards yet...", juce::dontSendNotification);
        noBoardsLabel.setColour(juce::Label::textColourId, juce::Colours::white);
        noBoardsLabel.setFont(juce::Font(18.0f));
        noBoardsLabel.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(noBoardsLabel);
    } else {
        for (int i = 0; i < savedBoards.size(); ++i) {
            auto name = savedBoards[i];
            
            // Load board button
            auto* btn = new juce::TextButton(name);
            btn->onClick = [this, name, i]() {
                if (onLoadBoard) {
                    // Find the board data for this name
                    BoardData* selectedBoard = nullptr;
                    for (auto& board : boardsData) {
                        if (board.name == name) {
                            selectedBoard = &board;
                            break;
                        }
                    }
                    
                    if (selectedBoard) {
                        // For now, just call with the name - we'll improve this later
                        onLoadBoard(name);
                    }
                }
            };
            boardButtons.add(btn);
            addAndMakeVisible(btn);
            
            // Delete button
            auto* deleteBtn = new juce::TextButton("Delete");
            deleteBtn->setColour(juce::TextButton::buttonColourId, juce::Colours::red);
            deleteBtn->setColour(juce::TextButton::textColourOffId, juce::Colours::white);
            deleteBtn->onClick = [this, name]() {
                deleteBoard(name);
            };
            deleteButtons.add(deleteBtn);
            addAndMakeVisible(deleteBtn);
        }
    }
}

void SavedBoardsScreen::paint(juce::Graphics& g)
{
    if (backgroundImage.isValid())
        g.drawImage(backgroundImage, getLocalBounds().toFloat());
    else
        g.fillAll(juce::Colours::darkcyan);
}

void SavedBoardsScreen::resized()
{
    backButton.setBounds(10, 10, 100, 35); 
    limitLabel.setBounds(getWidth() - 150, 10, 140, 35);
    
    if (!savedBoards.isEmpty()) {
        auto area = getLocalBounds().reduced(60);
        area.removeFromTop(60); // leave space for back button and limit label
        int buttonHeight = 40;
        int spacing = 20;
        int deleteButtonWidth = 80;
        int buttonSpacing = 10;
        
        for (int i = 0; i < boardButtons.size(); ++i)
        {
            auto rowArea = area.removeFromTop(buttonHeight);
            auto deleteArea = rowArea.removeFromRight(deleteButtonWidth);
            rowArea.removeFromRight(buttonSpacing); // spacing between buttons
            
            boardButtons[i]->setBounds(rowArea);
            deleteButtons[i]->setBounds(deleteArea);
            
            area.removeFromTop(spacing);
        }
    } else {
        // Leave space for the back button at the top
        noBoardsLabel.setBounds(getLocalBounds().withTrimmedTop(60));
    }
}


void SavedBoardsScreen::deleteBoard(const juce::String& boardName)
{
    auto* confirmAlert = new juce::AlertWindow("Delete Board", 
        "Are you sure you want to delete '" + boardName + "'?", 
        juce::AlertWindow::QuestionIcon);
    confirmAlert->addButton("Delete", 1);
    confirmAlert->addButton("Cancel", 0);
    
    confirmAlert->enterModalState(true, juce::ModalCallbackFunction::create(
        [this, confirmAlert, boardName](int result) {
            std::unique_ptr<juce::AlertWindow> cleanup(confirmAlert);
            if (result == 1) {
                // Delete from cloud
                auto& authManager = AuthManager::getInstance();
                if (authManager.isLoggedIn()) {
                    juce::String accessToken = authManager.getAccessToken();
                    juce::String userId = authManager.getCurrentUserId();
                    BoardResponse deleteResponse = supabaseClient.deleteBoard(accessToken, userId, boardName);
                    
                    if (!deleteResponse.success) {
                        auto* errorAlert = new juce::AlertWindow("Delete Failed", 
                            "Failed to delete board: " + deleteResponse.errorMessage, 
                            juce::AlertWindow::WarningIcon);
                        errorAlert->addButton("OK", 1);
                        errorAlert->enterModalState(true, juce::ModalCallbackFunction::create(
                            [errorAlert](int) {
                                std::unique_ptr<juce::AlertWindow> cleanup(errorAlert);
                            }), false);
                        return;
                    }
                }
                
                // Refresh the screen
                removeAllChildren();
                boardButtons.clear();
                deleteButtons.clear();
                savedBoards.clear();
                
                // Reinitialize the screen
                backButton.setButtonText("Home");
                backButton.setColour(juce::TextButton::buttonColourId, juce::Colours::grey);
                backButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
                backButton.onClick = [this]() { onBackToHome(); };
                addAndMakeVisible(backButton);
                
                
                loadBoards();
                
                if (savedBoards.isEmpty()) {
                    noBoardsLabel.setText("No saved boards yet...", juce::dontSendNotification);
                    noBoardsLabel.setColour(juce::Label::textColourId, juce::Colours::white);
                    noBoardsLabel.setFont(juce::Font(18.0f));
                    noBoardsLabel.setJustificationType(juce::Justification::centred);
                    addAndMakeVisible(noBoardsLabel);
                } else {
                    for (int i = 0; i < savedBoards.size(); ++i) {
                        auto name = savedBoards[i];
                        
                        auto* btn = new juce::TextButton(name);
                        btn->onClick = [this, name]() {
                            if (onLoadBoard)
                                onLoadBoard(name);
                        };
                        boardButtons.add(btn);
                        addAndMakeVisible(btn);
                        
                        auto* deleteBtn = new juce::TextButton("Delete");
                        deleteBtn->setColour(juce::TextButton::buttonColourId, juce::Colours::red);
                        deleteBtn->setColour(juce::TextButton::textColourOffId, juce::Colours::white);
                        deleteBtn->onClick = [this, name]() {
                            deleteBoard(name);
                        };
                        deleteButtons.add(deleteBtn);
                        addAndMakeVisible(deleteBtn);
                    }
                }
                
                resized();
                repaint();
            }
        }), false);
}

void SavedBoardsScreen::loadBoards()
{
    auto& authManager = AuthManager::getInstance();
    if (!authManager.isLoggedIn()) {
        return;
    }
    
    // Load boards from cloud
    juce::String accessToken = authManager.getAccessToken();
    BoardResponse response = supabaseClient.loadBoards(accessToken);
    
    if (response.success) {
        boardsData = response.boards;
        savedBoards.clear();
        for (const auto& board : boardsData) {
            savedBoards.add(board.name);
        }
    } else {
        // Show error message
        std::cout << "[SavedBoardsScreen] Failed to load boards: " << response.errorMessage << std::endl;
    }
}
