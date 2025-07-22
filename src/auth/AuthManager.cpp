#include "AuthManager.h"

AuthManager& AuthManager::getInstance() {
    static AuthManager instance;
    return instance;
}

AuthResponse AuthManager::signUp(const juce::String& email, const juce::String& password) {
    std::cout << "[AuthManager] Attempting sign up for: " << email << std::endl;
    
    AuthResponse response = supabaseClient.signUp(email, password);
    
    if (response.success) {
        // Store session data
        currentUserId = response.userId;
        currentUserEmail = response.email;
        accessToken = response.accessToken;
        
        // Set session expiry (30 days from now)
        sessionExpiry = juce::Time::getCurrentTime() + juce::RelativeTime::days(30);
        
        saveSession();
        std::cout << "[AuthManager] Sign up successful, session saved" << std::endl;
    }
    
    return response;
}

AuthResponse AuthManager::signIn(const juce::String& email, const juce::String& password) {
    std::cout << "[AuthManager] Attempting sign in for: " << email << std::endl;
    
    AuthResponse response = supabaseClient.signIn(email, password);
    
    if (response.success) {
        // Store session data
        currentUserId = response.userId;
        currentUserEmail = response.email;
        accessToken = response.accessToken;
        
        // Set session expiry (30 days from now)
        sessionExpiry = juce::Time::getCurrentTime() + juce::RelativeTime::days(30);
        
        saveSession();
        std::cout << "[AuthManager] Sign in successful, session saved" << std::endl;
    }
    
    return response;
}

void AuthManager::signOut() {
    std::cout << "[AuthManager] Signing out user: " << currentUserEmail << std::endl;
    
    supabaseClient.signOut();
    clearSession();
}

bool AuthManager::isLoggedIn() const {
    return !currentUserId.isEmpty() && !accessToken.isEmpty() && isSessionValid();
}

juce::String AuthManager::getCurrentUserId() const {
    return currentUserId;
}

juce::String AuthManager::getCurrentUserEmail() const {
    return currentUserEmail;
}

juce::File AuthManager::getUserBoardsDirectory() const {
    if (!isLoggedIn()) {
        return juce::File::getCurrentWorkingDirectory().getChildFile("Boards");
    }
    
    return juce::File::getCurrentWorkingDirectory()
           .getChildFile("UserBoards")
           .getChildFile(currentUserId);
}

juce::File AuthManager::getUserBoardFile(const juce::String& boardName) const {
    return getUserBoardsDirectory().getChildFile(boardName + ".json");
}

void AuthManager::saveSession() {
    auto sessionFile = getSessionFile();
    
    juce::DynamicObject::Ptr sessionData = new juce::DynamicObject();
    sessionData->setProperty("userId", currentUserId);
    sessionData->setProperty("email", currentUserEmail);
    sessionData->setProperty("accessToken", accessToken);
    sessionData->setProperty("expiry", (juce::int64) sessionExpiry.toMilliseconds());
    
    juce::var sessionVar(sessionData);
    juce::String sessionJson = juce::JSON::toString(sessionVar);
    
    if (sessionFile.replaceWithText(sessionJson)) {
        std::cout << "[AuthManager] Session saved successfully" << std::endl;
    } else {
        std::cout << "[AuthManager] ERROR: Failed to save session" << std::endl;
    }
}

void AuthManager::loadSession() {
    auto sessionFile = getSessionFile();
    
    if (!sessionFile.existsAsFile()) {
        std::cout << "[AuthManager] No existing session found" << std::endl;
        return;
    }
    
    juce::String sessionJson = sessionFile.loadFileAsString();
    if (sessionJson.isEmpty()) {
        std::cout << "[AuthManager] Session file is empty" << std::endl;
        return;
    }
    
    juce::var parsedSession;
    auto result = juce::JSON::parse(sessionJson, parsedSession);
    
    if (!result.wasOk()) {
        std::cout << "[AuthManager] ERROR: Invalid session file format" << std::endl;
        clearSession();
        return;
    }
    
    auto* sessionObject = parsedSession.getDynamicObject();
    if (!sessionObject) {
        std::cout << "[AuthManager] ERROR: Malformed session data" << std::endl;
        clearSession();
        return;
    }
    
    // Load session data
    currentUserId = sessionObject->getProperty("userId").toString();
    currentUserEmail = sessionObject->getProperty("email").toString();
    accessToken = sessionObject->getProperty("accessToken").toString();
    
    juce::int64 expiryMs = sessionObject->getProperty("expiry");
    sessionExpiry = juce::Time(expiryMs);
    
    if (isSessionValid()) {
        std::cout << "[AuthManager] Session loaded successfully for: " << currentUserEmail << std::endl;
    } else {
        std::cout << "[AuthManager] Session expired, clearing" << std::endl;
        clearSession();
    }
}

void AuthManager::clearSession() {
    currentUserId.clear();
    currentUserEmail.clear();
    accessToken.clear();
    sessionExpiry = juce::Time();
    
    auto sessionFile = getSessionFile();
    if (sessionFile.existsAsFile()) {
        sessionFile.deleteFile();
        std::cout << "[AuthManager] Session file deleted" << std::endl;
    }
}

juce::File AuthManager::getSessionFile() const {
    return juce::File::getCurrentWorkingDirectory().getChildFile(".session");
}

bool AuthManager::isSessionValid() const {
    if (sessionExpiry == juce::Time()) {
        return false;
    }
    
    return juce::Time::getCurrentTime() < sessionExpiry;
}

juce::String AuthManager::getAccessToken() const {
    return accessToken;
}