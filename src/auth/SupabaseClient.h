#pragma once

#include <JuceHeader.h>

// Forward declaration to avoid circular dependency
class AuthManager;

struct AuthResponse {
    bool success = false;
    juce::String errorMessage;
    juce::String userId;
    juce::String accessToken;
    juce::String email;
};

struct BoardData {
    juce::String id;
    juce::String name;
    juce::var boardData;
    juce::String createdAt;
    juce::String updatedAt;
};

struct BoardResponse {
    bool success = false;
    juce::String errorMessage;
    juce::Array<BoardData> boards;
};

class SupabaseClient {
public:
    SupabaseClient();
    
    // Authentication methods
    AuthResponse signUp(const juce::String& email, const juce::String& password);
    AuthResponse signIn(const juce::String& email, const juce::String& password);
    void signOut();
    
    // Board management methods
    BoardResponse saveBoard(const juce::String& accessToken, const juce::String& userId, const juce::String& boardName, const juce::var& boardData);
    BoardResponse loadBoards(const juce::String& accessToken);
    BoardResponse deleteBoard(const juce::String& accessToken, const juce::String& userId, const juce::String& boardName);
    
    // Utility methods
    bool isValidEmail(const juce::String& email);
    bool isValidPassword(const juce::String& password);
    
private:
    juce::String supabaseUrl;
    juce::String supabaseKey;
    
    // HTTP request helpers
    juce::String makeRequest(const juce::String& method, 
                           const juce::String& endpoint, 
                           const juce::String& jsonBody = "",
                           const juce::String& accessToken = "");
    
    // Response parsing methods
    AuthResponse parseAuthResponse(const juce::String& responseJson);
    BoardResponse parseBoardResponse(const juce::String& responseJson);
    
    // Load credentials from environment/config
    void loadCredentials();
};