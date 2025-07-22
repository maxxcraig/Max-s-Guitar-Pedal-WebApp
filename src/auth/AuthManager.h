#pragma once

#include <JuceHeader.h>
#include "SupabaseClient.h"

class AuthManager {
public:
    static AuthManager& getInstance();
    
    // Authentication methods
    AuthResponse signUp(const juce::String& email, const juce::String& password);
    AuthResponse signIn(const juce::String& email, const juce::String& password);
    void signOut();
    
    // Session management
    bool isLoggedIn() const;
    juce::String getCurrentUserId() const;
    juce::String getCurrentUserEmail() const;
    juce::String getAccessToken() const;
    
    // User-specific file paths
    juce::File getUserBoardsDirectory() const;
    juce::File getUserBoardFile(const juce::String& boardName) const;
    
    // Session management
    void loadSession();
    
private:
    AuthManager() = default;
    ~AuthManager() = default;
    
    // Prevent copying
    AuthManager(const AuthManager&) = delete;
    AuthManager& operator=(const AuthManager&) = delete;
    
    SupabaseClient supabaseClient;
    
    // Session data
    juce::String currentUserId;
    juce::String currentUserEmail;
    juce::String accessToken;
    juce::Time sessionExpiry;
    
    // Session persistence
    void saveSession();
    void clearSession();
    juce::File getSessionFile() const;
    
    // Session validation
    bool isSessionValid() const;
};