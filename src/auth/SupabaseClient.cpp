#include "SupabaseClient.h"

SupabaseClient::SupabaseClient() {
    loadCredentials();
}

void SupabaseClient::loadCredentials() {
    // Load credentials from app bundle or working directory
    // Try app bundle first (for distribution), then working directory (for development)
    
    juce::File envFile;
    
    // First try app bundle path (for distributed apps)
    auto appFile = juce::File::getSpecialLocation(juce::File::currentApplicationFile);
    envFile = appFile.getChildFile("Contents/Resources/.env");
    
    // If not found, try working directory (for development)
    if (!envFile.existsAsFile()) {
        envFile = juce::File::getCurrentWorkingDirectory().getChildFile(".env");
    }
    
    if (envFile.existsAsFile()) {
        auto envContent = envFile.loadFileAsString();
        auto lines = juce::StringArray::fromLines(envContent);
        
        for (auto& line : lines) {
            if (line.startsWith("SUPABASE_URL=")) {
                supabaseUrl = line.substring(13); // Remove "SUPABASE_URL="
            } else if (line.startsWith("SUPABASE_ANON_KEY=")) {
                supabaseKey = line.substring(18); // Remove "SUPABASE_ANON_KEY="
            }
        }
    }
    
    // Secure fallback - use embedded public credentials for demo
    if (supabaseUrl.isEmpty() || supabaseKey.isEmpty()) {
        std::cout << "[SupabaseClient] Using embedded demo credentials" << std::endl;
        // These are the same values from your .env - the anon key is safe to distribute
        supabaseUrl = "https://xahjewffithramizvmnr.supabase.co";
        supabaseKey = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiJzdXBhYmFzZSIsInJlZiI6InhhaGpld2ZmaXRocmFtaXp2bW5yIiwicm9sZSI6ImFub24iLCJpYXQiOjE3NTExNDYxMTYsImV4cCI6MjA2NjcyMjExNn0.UIH5aMsvYe4mWLWcohcPymXZ23VW2jzcIhjSu6JwAe8";
    }
    
    std::cout << "[SupabaseClient] Initialized with URL: " << supabaseUrl << std::endl;
}

AuthResponse SupabaseClient::signUp(const juce::String& email, const juce::String& password) {
    std::cout << "[SupabaseClient] Attempting sign up for: " << email << std::endl;
    
    AuthResponse response;
    
    // Validate input
    if (!isValidEmail(email)) {
        response.errorMessage = "Invalid email format";
        return response;
    }
    
    if (!isValidPassword(password)) {
        response.errorMessage = "Password must be at least 6 characters";
        return response;
    }
    
    // Create JSON body for Supabase auth API
    juce::DynamicObject::Ptr jsonData = new juce::DynamicObject();
    jsonData->setProperty("email", email);
    jsonData->setProperty("password", password);
    
    juce::var jsonVar(jsonData);
    juce::String jsonBody = juce::JSON::toString(jsonVar);
    
    // Make request to Supabase
    juce::String responseJson = makeRequest("POST", "/auth/v1/signup", jsonBody);
    
    if (responseJson.isEmpty()) {
        response.errorMessage = "Network error - could not connect to authentication server";
        return response;
    }
    
    return parseAuthResponse(responseJson);
}

AuthResponse SupabaseClient::signIn(const juce::String& email, const juce::String& password) {
    std::cout << "[SupabaseClient] Attempting sign in for: " << email << std::endl;
    
    AuthResponse response;
    
    // Validate input
    if (!isValidEmail(email)) {
        response.errorMessage = "Invalid email format";
        return response;
    }
    
    if (password.isEmpty()) {
        response.errorMessage = "Password cannot be empty";
        return response;
    }
    
    // Create JSON body for Supabase auth API
    juce::DynamicObject::Ptr jsonData = new juce::DynamicObject();
    jsonData->setProperty("email", email);
    jsonData->setProperty("password", password);
    
    juce::var jsonVar(jsonData);
    juce::String jsonBody = juce::JSON::toString(jsonVar);
    
    // Make request to Supabase
    juce::String responseJson = makeRequest("POST", "/auth/v1/token?grant_type=password", jsonBody);
    
    if (responseJson.isEmpty()) {
        response.errorMessage = "Network error - could not connect to authentication server";
        return response;
    }
    
    return parseAuthResponse(responseJson);
}

void SupabaseClient::signOut() {
    std::cout << "[SupabaseClient] User signed out" << std::endl;
}

juce::String SupabaseClient::makeRequest(const juce::String& method, 
                                        const juce::String& endpoint, 
                                        const juce::String& jsonBody,
                                        const juce::String& accessToken) {
    
    juce::String fullUrl = supabaseUrl + endpoint;
    std::cout << "[SupabaseClient] Making " << method << " request to: " << fullUrl << std::endl;
    
    try {
        if (method == "POST") {
            std::cout << "[SupabaseClient] POST request with body: " << jsonBody.substring(0, 100) << "..." << std::endl;
            
            // Create URL with parameters
            juce::URL url(fullUrl);
            
            // Prepare headers string manually
            juce::String headerString = "Content-Type: application/json\r\n";
            headerString += "apikey: " + supabaseKey + "\r\n";
            
            // Use access token if provided (for authenticated requests), otherwise use anon key
            if (accessToken.isNotEmpty()) {
                headerString += "Authorization: Bearer " + accessToken + "\r\n";
            } else {
                headerString += "Authorization: Bearer " + supabaseKey + "\r\n";
            }
            
            // Use JUCE's URL with POST data
            juce::URL urlWithData = url.withPOSTData(jsonBody);
            
            auto inputStream = urlWithData.createInputStream(
                juce::URL::InputStreamOptions(juce::URL::ParameterHandling::inAddress)
                .withExtraHeaders(headerString)
                .withConnectionTimeoutMs(10000)
            );
            
            if (inputStream != nullptr) {
                juce::String response = inputStream->readEntireStreamAsString();
                std::cout << "[SupabaseClient] Response received (" << response.length() << " chars): " << response.substring(0, 200) << "..." << std::endl;
                return response;
            } else {
                std::cout << "[SupabaseClient] ERROR: Failed to create input stream" << std::endl;
                return "{\"error\":{\"message\":\"Failed to connect to authentication server\"}}";
            }
        } else if (method == "GET") {
            juce::URL url(fullUrl);
            
            // Prepare headers string manually
            juce::String headerString = "apikey: " + supabaseKey + "\r\n";
            
            // Use access token if provided (for authenticated requests), otherwise use anon key
            if (accessToken.isNotEmpty()) {
                headerString += "Authorization: Bearer " + accessToken + "\r\n";
            } else {
                headerString += "Authorization: Bearer " + supabaseKey + "\r\n";
            }
            
            auto inputStream = url.createInputStream(
                juce::URL::InputStreamOptions(juce::URL::ParameterHandling::inAddress)
                .withExtraHeaders(headerString)
                .withConnectionTimeoutMs(10000)
            );
            
            if (inputStream != nullptr) {
                juce::String response = inputStream->readEntireStreamAsString();
                std::cout << "[SupabaseClient] GET Response received (" << response.length() << " chars): " << response.substring(0, 200) << "..." << std::endl;
                return response;
            } else {
                std::cout << "[SupabaseClient] ERROR: Failed to create input stream for GET" << std::endl;
                return "{\"error\":{\"message\":\"Failed to connect to server\"}}";
            }
        } else if (method == "DELETE") {
            juce::URL url(fullUrl);
            
            // Prepare headers string manually
            juce::String headerString = "apikey: " + supabaseKey + "\r\n";
            
            // Use access token if provided (for authenticated requests), otherwise use anon key
            if (accessToken.isNotEmpty()) {
                headerString += "Authorization: Bearer " + accessToken + "\r\n";
            } else {
                headerString += "Authorization: Bearer " + supabaseKey + "\r\n";
            }
            
            // For DELETE requests, we need to use a different approach
            juce::URL urlWithData = url.withPOSTData(""); // Empty body for DELETE
            
            auto inputStream = urlWithData.createInputStream(
                juce::URL::InputStreamOptions(juce::URL::ParameterHandling::inAddress)
                .withExtraHeaders(headerString + "X-HTTP-Method-Override: DELETE\r\n")
                .withConnectionTimeoutMs(10000)
            );
            
            if (inputStream != nullptr) {
                juce::String response = inputStream->readEntireStreamAsString();
                std::cout << "[SupabaseClient] DELETE Response received (" << response.length() << " chars): " << response.substring(0, 200) << "..." << std::endl;
                return response;
            } else {
                std::cout << "[SupabaseClient] ERROR: Failed to create input stream for DELETE" << std::endl;
                return "{\"error\":{\"message\":\"Failed to connect to server\"}}";
            }
        }
        
        std::cout << "[SupabaseClient] ERROR: Unsupported method: " << method << std::endl;
        return "{\"error\":{\"message\":\"Unsupported HTTP method\"}}";
        
    } catch (const std::exception& e) {
        std::cout << "[SupabaseClient] ERROR: Exception during request: " << e.what() << std::endl;
        return "{\"error\":{\"message\":\"Network connection error\"}}";
    }
}

AuthResponse SupabaseClient::parseAuthResponse(const juce::String& responseJson) {
    AuthResponse response;
    
    try {
        juce::var parsedJson;
        auto result = juce::JSON::parse(responseJson, parsedJson);
        
        if (!result.wasOk()) {
            response.errorMessage = "Invalid response from server";
            return response;
        }
        
        auto* jsonObject = parsedJson.getDynamicObject();
        if (!jsonObject) {
            response.errorMessage = "Malformed response from server";
            return response;
        }
        
        // Check for error in response (multiple formats)
        if (jsonObject->hasProperty("error")) {
            auto errorObj = jsonObject->getProperty("error");
            if (auto* errorDynamic = errorObj.getDynamicObject()) {
                response.errorMessage = errorDynamic->getProperty("message").toString();
            } else {
                response.errorMessage = errorObj.toString();
            }
            return response;
        }
        
        // Check for Supabase error format with error_code
        if (jsonObject->hasProperty("error_code")) {
            juce::String errorCode = jsonObject->getProperty("error_code").toString();
            juce::String errorMsg = jsonObject->getProperty("msg").toString();
            
            if (errorCode == "email_not_confirmed") {
                response.errorMessage = "Please check your email and click the confirmation link before signing in.";
            } else {
                response.errorMessage = errorMsg.isNotEmpty() ? errorMsg : "Authentication error: " + errorCode;
            }
            return response;
        }
        
        // Check for success indicators
        // Supabase signup returns user data directly, signin returns access_token + user
        if (jsonObject->hasProperty("access_token") || jsonObject->hasProperty("user") || jsonObject->hasProperty("id")) {
            response.success = true;
            
            // Extract access token (for signin)
            if (jsonObject->hasProperty("access_token")) {
                response.accessToken = jsonObject->getProperty("access_token").toString();
            }
            
            // Extract user information from user object (signin)
            if (jsonObject->hasProperty("user")) {
                auto userObj = jsonObject->getProperty("user");
                if (auto* userDynamic = userObj.getDynamicObject()) {
                    response.userId = userDynamic->getProperty("id").toString();
                    response.email = userDynamic->getProperty("email").toString();
                }
            }
            // Extract user information directly from response (signup)
            else if (jsonObject->hasProperty("id") && jsonObject->hasProperty("email")) {
                response.userId = jsonObject->getProperty("id").toString();
                response.email = jsonObject->getProperty("email").toString();
            }
            
            std::cout << "[SupabaseClient] Authentication successful for user: " << response.email << " (ID: " << response.userId << ")" << std::endl;
            return response;
        }
        
        // If we get here, we have an unexpected response format
        response.errorMessage = "Unexpected response format from server";
        return response;
        
    } catch (const std::exception& e) {
        response.errorMessage = "Error parsing server response: " + juce::String(e.what());
        return response;
    }
}

bool SupabaseClient::isValidEmail(const juce::String& email) {
    // Basic email validation
    return email.contains("@") && 
           email.contains(".") && 
           email.length() > 5 &&
           !email.startsWith("@") &&
           !email.endsWith("@");
}

bool SupabaseClient::isValidPassword(const juce::String& password) {
    // Supabase requires minimum 6 characters
    return password.length() >= 6;
}

BoardResponse SupabaseClient::saveBoard(const juce::String& accessToken, const juce::String& userId, const juce::String& boardName, const juce::var& boardData) {
    std::cout << "[SupabaseClient] Saving board: " << boardName << " for user: " << userId << std::endl;
    
    BoardResponse response;
    
    if (accessToken.isEmpty()) {
        response.errorMessage = "User not authenticated";
        return response;
    }
    
    if (userId.isEmpty()) {
        response.errorMessage = "User ID is required";
        return response;
    }
    
    if (boardName.isEmpty()) {
        response.errorMessage = "Board name cannot be empty";
        return response;
    }
    
    // Create JSON body for board data - explicitly include user_id
    juce::DynamicObject::Ptr jsonData = new juce::DynamicObject();
    jsonData->setProperty("user_id", userId);
    jsonData->setProperty("name", boardName);
    jsonData->setProperty("board_data", boardData);
    
    juce::var jsonVar(jsonData);
    juce::String jsonBody = juce::JSON::toString(jsonVar);
    
    // Use upsert (insert or update) endpoint
    juce::String endpoint = "/rest/v1/user_boards";
    juce::String responseJson = makeRequest("POST", endpoint, jsonBody, accessToken);
    
    if (responseJson.isEmpty()) {
        // Empty response from Supabase POST is actually success (no data returned by default)
        response.success = true;
        std::cout << "[SupabaseClient] Board saved successfully (empty response indicates success)" << std::endl;
        return response;
    }
    
    return parseBoardResponse(responseJson);
}

BoardResponse SupabaseClient::loadBoards(const juce::String& accessToken) {
    std::cout << "[SupabaseClient] Loading boards for authenticated user with token: " << accessToken.substring(0, 20) << "..." << std::endl;
    
    BoardResponse response;
    
    if (accessToken.isEmpty()) {
        response.errorMessage = "User not authenticated";
        return response;
    }
    
    // Get all boards for the authenticated user
    juce::String endpoint = "/rest/v1/user_boards?select=*&order=created_at.desc";
    juce::String responseJson = makeRequest("GET", endpoint, "", accessToken);
    
    if (responseJson.isEmpty()) {
        response.errorMessage = "Network error - could not connect to server";
        return response;
    }
    
    return parseBoardResponse(responseJson);
}

BoardResponse SupabaseClient::deleteBoard(const juce::String& accessToken, const juce::String& userId, const juce::String& boardName) {
    std::cout << "[SupabaseClient] Deleting board: " << boardName << " for user: " << userId << std::endl;
    
    BoardResponse response;
    
    if (accessToken.isEmpty()) {
        response.errorMessage = "User not authenticated";
        return response;
    }
    
    if (userId.isEmpty()) {
        response.errorMessage = "User ID is required";
        return response;
    }
    
    if (boardName.isEmpty()) {
        response.errorMessage = "Board name cannot be empty";
        return response;
    }
    
    // Delete board by name AND user_id for the authenticated user (double security)
    juce::String endpoint = "/rest/v1/user_boards?name=eq." + juce::URL::addEscapeChars(boardName, false) + "&user_id=eq." + userId;
    juce::String responseJson = makeRequest("DELETE", endpoint, "", accessToken);
    
    if (responseJson.isEmpty()) {
        response.errorMessage = "Network error - could not connect to server";
        return response;
    }
    
    return parseBoardResponse(responseJson);
}

BoardResponse SupabaseClient::parseBoardResponse(const juce::String& responseJson) {
    BoardResponse response;
    
    try {
        juce::var parsedJson;
        auto result = juce::JSON::parse(responseJson, parsedJson);
        
        if (!result.wasOk()) {
            response.errorMessage = "Invalid response from server";
            return response;
        }
        
        // Check if it's an error response
        if (auto* errorObj = parsedJson.getDynamicObject()) {
            if (errorObj->hasProperty("error")) {
                auto errorData = errorObj->getProperty("error");
                if (auto* errorDynamic = errorData.getDynamicObject()) {
                    response.errorMessage = errorDynamic->getProperty("message").toString();
                } else {
                    response.errorMessage = errorData.toString();
                }
                return response;
            }
            
            // Check for Supabase error format
            if (errorObj->hasProperty("code")) {
                response.errorMessage = errorObj->getProperty("message").toString();
                return response;
            }
        }
        
        // Parse successful response
        response.success = true;
        
        // Handle array response (for loadBoards)
        if (auto* boardArray = parsedJson.getArray()) {
            for (auto& boardEntry : *boardArray) {
                if (auto* boardObj = boardEntry.getDynamicObject()) {
                    BoardData board;
                    board.id = boardObj->getProperty("id").toString();
                    board.name = boardObj->getProperty("name").toString();
                    board.boardData = boardObj->getProperty("board_data");
                    board.createdAt = boardObj->getProperty("created_at").toString();
                    board.updatedAt = boardObj->getProperty("updated_at").toString();
                    response.boards.add(board);
                }
            }
        }
        // Handle single object response (for saveBoard)
        else if (auto* boardObj = parsedJson.getDynamicObject()) {
            if (boardObj->hasProperty("id")) {
                BoardData board;
                board.id = boardObj->getProperty("id").toString();
                board.name = boardObj->getProperty("name").toString();
                board.boardData = boardObj->getProperty("board_data");
                board.createdAt = boardObj->getProperty("created_at").toString();
                board.updatedAt = boardObj->getProperty("updated_at").toString();
                response.boards.add(board);
            }
        }
        
        std::cout << "[SupabaseClient] Board operation successful, returned " << response.boards.size() << " boards" << std::endl;
        return response;
        
    } catch (const std::exception& e) {
        response.errorMessage = "Error parsing server response: " + juce::String(e.what());
        return response;
    }
}