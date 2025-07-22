#pragma once
#include <JuceHeader.h>
#include "screens/SavedBoardsScreen.h"
#include "auth/SupabaseClient.h"

class ScreenRouter : public juce::Component
{
public:
    ScreenRouter();

    void showLoginScreen();
    void showHomeScreen();
    void showPedalBoardScreen();
    void showAboutScreen();
    void clearScreen();
    void showCreateAccountScreen();
    void showSavedBoardsScreen();
    void showPedalBoardScreenFromSave(const juce::String& boardName);
    void showPedalBoardScreenWithData(const juce::var& boardData);
    void handleLogout();

    void resized() override;

private:
    std::unique_ptr<juce::Component> currentScreen;
    SupabaseClient supabaseClient;
};
