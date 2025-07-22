#include <JuceHeader.h>
#include "ScreenRouter.h"
#include "screens/LoginScreen.h"
#include "screens/HomeScreen.h"
#include "screens/AboutScreen.h"
#include "screens/PedalBoardScreen.h"
#include "screens/CreateAccountScreen.h"
#include "screens/SavedBoardsScreen.h"
#include "auth/AuthManager.h"

ScreenRouter::ScreenRouter()
{
    setSize(1000, 600);
    
    // Load any existing session and go to appropriate screen
    auto& authManager = AuthManager::getInstance();
    authManager.loadSession();
    
    if (authManager.isLoggedIn()) {
        showHomeScreen();
    } else {
        showLoginScreen();
    }
}

void ScreenRouter::showLoginScreen()
{
    clearScreen();
    currentScreen.reset(new LoginScreen(
    [this]() { showHomeScreen(); },
    [this]() { showCreateAccountScreen(); }
    ));

    addAndMakeVisible(*currentScreen);
    resized();
}

void ScreenRouter::showHomeScreen()
{
    clearScreen();
    currentScreen.reset(new HomeScreen(
    [this]() { showPedalBoardScreen(); },
    [this]() { showAboutScreen(); },
    [this]() { handleLogout(); }, // logout
    [this]() { showSavedBoardsScreen(); } // view saved
    ));
    addAndMakeVisible(*currentScreen);
    resized();
}

void ScreenRouter::showPedalBoardScreen()
{
    clearScreen();
    currentScreen.reset(new PedalBoardScreen([this]() {
        showHomeScreen();
    }));
    addAndMakeVisible(*currentScreen);
    resized();
}
void ScreenRouter::showCreateAccountScreen()
{
    clearScreen();

    auto* createAccountScreen = new CreateAccountScreen(
        [this]() { showLoginScreen(); },  // back to login
        [this]() { showHomeScreen(); }    // account created successfully
    );

    addAndMakeVisible(createAccountScreen);
    currentScreen.reset(createAccountScreen);
    resized(); 
}


void ScreenRouter::showSavedBoardsScreen()
{
    clearScreen();
    currentScreen = std::make_unique<SavedBoardsScreen>(
        [this]() { showHomeScreen(); },
        [this](const juce::String& boardName) {
            showPedalBoardScreenFromSave(boardName);
        });

    addAndMakeVisible(*currentScreen);
    resized();
}






void ScreenRouter::showAboutScreen()
{
    clearScreen();
    currentScreen.reset(new AboutScreen([this]() {
        showHomeScreen();
    }));
    addAndMakeVisible(*currentScreen);
    resized();
}

void ScreenRouter::showPedalBoardScreenFromSave(const juce::String& boardName)
{
    std::cout << "[ScreenRouter] showPedalBoardScreenFromSave called for: " << boardName << std::endl;
    
    // Load board data from cloud
    juce::var boardVar;
    auto& authManager = AuthManager::getInstance();
    
    if (authManager.isLoggedIn()) {
        juce::String accessToken = authManager.getAccessToken();
        BoardResponse response = supabaseClient.loadBoards(accessToken);
        
        if (response.success) {
            // Find the board with matching name
            for (const auto& board : response.boards) {
                if (board.name == boardName) {
                    boardVar = board.boardData;
                    break;
                }
            }
        }
    }

    // Check if we already have a PedalBoardScreen
    if (auto* existingScreen = dynamic_cast<PedalBoardScreen*>(currentScreen.get())) {
        std::cout << "[ScreenRouter] reusing existing PedalBoardScreen" << std::endl;
        if (!boardVar.isVoid()) {
            std::cout << "[ScreenRouter] calling loadBoardData..." << std::endl;
            existingScreen->loadBoardData(boardVar);
            std::cout << "[ScreenRouter] finished loadBoardData call" << std::endl;
        }
        return;
    }
    
    // Otherwise, create a new screen
    clearScreen();
    juce::Thread::sleep(50); // Give time for audio cleanup
    
    auto* screen = new PedalBoardScreen([this]() { showHomeScreen(); });
    currentScreen.reset(screen);
    addAndMakeVisible(*currentScreen);
    resized();

    // Now load the data into the pedal board
    if (!boardVar.isVoid()) {
        if (auto* pedalBoardScreen = dynamic_cast<PedalBoardScreen*>(currentScreen.get())) {
            std::cout << "[ScreenRouter] calling loadBoardData..." << std::endl;
            pedalBoardScreen->loadBoardData(boardVar);
            std::cout << "[ScreenRouter] finished loadBoardData call" << std::endl;
        }
    }
}

void ScreenRouter::showPedalBoardScreenWithData(const juce::var& boardData)
{
    std::cout << "[ScreenRouter] showPedalBoardScreenWithData called" << std::endl;
    
    // Check if we already have a PedalBoardScreen
    if (auto* existingScreen = dynamic_cast<PedalBoardScreen*>(currentScreen.get())) {
        std::cout << "[ScreenRouter] reusing existing PedalBoardScreen" << std::endl;
        if (!boardData.isVoid()) {
            std::cout << "[ScreenRouter] calling loadBoardData..." << std::endl;
            existingScreen->loadBoardData(boardData);
            std::cout << "[ScreenRouter] finished loadBoardData call" << std::endl;
        }
        return;
    }
    
    // Otherwise, create a new screen
    clearScreen();
    juce::Thread::sleep(50); // Give time for audio cleanup
    
    auto* screen = new PedalBoardScreen([this]() { showHomeScreen(); });
    currentScreen.reset(screen);
    addAndMakeVisible(*currentScreen);
    resized();

    // Now load the data into the pedal board
    if (!boardData.isVoid()) {
        if (auto* pedalBoardScreen = dynamic_cast<PedalBoardScreen*>(currentScreen.get())) {
            std::cout << "[ScreenRouter] calling loadBoardData..." << std::endl;
            pedalBoardScreen->loadBoardData(boardData);
            std::cout << "[ScreenRouter] finished loadBoardData call" << std::endl;
        }
    }
}

void ScreenRouter::clearScreen()
{
    if (currentScreen)
        removeChildComponent(currentScreen.get());
    currentScreen.reset();
}

void ScreenRouter::resized()
{
    if (currentScreen)
        currentScreen->setBounds(getLocalBounds());
}

void ScreenRouter::handleLogout()
{
    auto& authManager = AuthManager::getInstance();
    authManager.signOut();
    showLoginScreen();
}
