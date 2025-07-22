#pragma once

#include <JuceHeader.h>
#include "auth/SupabaseClient.h"
#include "auth/AuthManager.h"

class SavedBoardsScreen : public juce::Component {
public:
    SavedBoardsScreen(std::function<void()> goHome,
                      std::function<void(const juce::String&)> loadBoard);

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    void loadBoards();
    void deleteBoard(const juce::String& boardName);

    std::function<void()> onBackToHome;
    std::function<void(const juce::String&)> onLoadBoard;

    juce::TextButton backButton;
    juce::Label noBoardsLabel;
    juce::Label limitLabel;

    juce::OwnedArray<juce::TextButton> boardButtons;
    juce::OwnedArray<juce::TextButton> deleteButtons;
    juce::StringArray savedBoards;
    juce::Array<BoardData> boardsData; // Store full board data from cloud

    // Cloud storage
    SupabaseClient supabaseClient;

    juce::Image backgroundImage;
};
