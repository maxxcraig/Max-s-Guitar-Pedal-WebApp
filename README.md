# Guitar Pedal Simulator App

A standalone JUCE-based guitar pedal simulator with cloud authentication and board management capabilities. This application demonstrates modern C++ development practices, JUCE framework integration, and cloud service integration for a resume-worthy project.

## Features

- **Real-time Guitar Effects**: Overdrive, reverb, chorus, and more pedal effects
- **Visual Pedal Board**: Drag-and-drop interface for creating custom pedal chains
- **Cloud Authentication**: Supabase-powered user accounts and session management
- **Board Persistence**: Save and load custom pedal board configurations
- **User-specific Storage**: Each user has their own private board collection

## Quick Start

### Prerequisites
- CMake 3.20 or higher
- Modern C++ compiler with C++17 support
- JUCE framework (included as submodule)
- Supabase account (for authentication features)

### Setup Instructions

1. **Clone the repository**:
   ```bash
   git clone <repository-url>
   cd Guitar-Pedal-App
   git submodule update --init --recursive
   ```

2. **Configure Supabase Authentication**:
   - Go to [supabase.com](https://supabase.com) and create a new project
   - In your project dashboard, navigate to Settings > API
   - Copy the `.env.example` file to `.env`:
     ```bash
     cp .env.example .env
     ```
   - Edit `.env` and add your Supabase credentials:
     ```
     SUPABASE_URL=your-project-url.supabase.co
     SUPABASE_ANON_KEY=your-anon-key-here
     ```

3. **Build the application**:
   ```bash
   mkdir build && cd build
   cmake ..
   cmake --build .
   ```

4. **Run the application**:
   ```bash
   ./GuitarPedalApp
   ```

## Architecture Overview

### Core Components

- **Main Application** (`src/main.cpp`): Entry point and JUCE application setup
- **Screen Router** (`src/ScreenRouter.cpp`): Navigation and screen management
- **Authentication System** (`src/auth/`): Supabase integration and session management
- **Pedal Effects** (`src/pedalSoundEffects/`): DSP processing for guitar effects
- **GUI Components** (`src/pedalGUI/`): Visual pedal representations
- **Screens** (`src/screens/`): Login, board management, and main interface

### Authentication Flow

1. **Login/Signup**: Users authenticate through Supabase REST API
2. **Session Management**: JWT tokens stored securely with 30-day expiration
3. **User-specific Storage**: Board configurations saved per-user in `UserBoards/[user-id]/`
4. **Automatic Session Restore**: Login state persists across app restarts

### Security Features

- Environment variable credential management (`.env` files)
- JWT token authentication with expiration
- Sensitive data excluded from version control (`.gitignore`)
- Input validation for email and password fields

## Development Details

### Technology Stack
- **Framework**: JUCE 7+ (C++ audio application framework)
- **Authentication**: Supabase REST API
- **Build System**: CMake
- **Language**: C++17
- **Platform**: Cross-platform (macOS, Windows, Linux)

### Project Structure
```
Guitar-Pedal-App/
├── src/
│   ├── auth/                 # Authentication system
│   │   ├── SupabaseClient.h/.cpp  # HTTP API client
│   │   └── AuthManager.h/.cpp     # Session management
│   ├── pedalGUI/             # Visual components
│   ├── pedalSoundEffects/    # Audio processing
│   ├── screens/              # UI screens
│   └── main.cpp              # Application entry
├── external/JUCE/            # JUCE framework
├── resources/                # Assets and images
├── .env.example              # Environment template
├── .gitignore               # Git exclusions
└── CMakeLists.txt           # Build configuration
```

### Key Implementation Details

- **HTTP Requests**: JUCE's `URL` and `WebInputStream` for Supabase API calls
- **JSON Processing**: JUCE's `JSON` and `DynamicObject` for API responses
- **File I/O**: JUCE's `File` class for session and board persistence
- **Error Handling**: Comprehensive validation and user feedback
- **Memory Management**: JUCE smart pointers and RAII patterns

## Configuration

### Supabase Setup
1. Create a Supabase project at [supabase.com](https://supabase.com)
2. Authentication is handled through the built-in auth system
3. For development, you may want to disable email confirmation in:
   `Authentication > Settings > Email Auth > Confirm email`

### Environment Variables
The application uses `.env` files for configuration:
- `SUPABASE_URL`: Your project's API URL
- `SUPABASE_ANON_KEY`: Your project's anonymous/public key

**Important**: Never commit `.env` files to version control.

## Building and Deployment

### Build Requirements
- CMake 3.20+
- C++17 compatible compiler
- JUCE dependencies (included as submodule)

### Build Commands
```bash
# Configure
cmake -B build -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build build --config Release

# Install (optional)
cmake --install build
```

## Contributing

This project demonstrates professional C++ development practices:
- Modern CMake build system
- JUCE framework integration
- Cloud service authentication
- Secure credential management
- Cross-platform compatibility

## Resume Highlights

This project showcases:
- **Modern C++**: C++17 features, RAII, smart pointers
- **Framework Integration**: JUCE audio framework expertise
- **Cloud Services**: REST API integration with Supabase
- **Security Best Practices**: Environment variables, JWT tokens
- **Audio Programming**: Real-time DSP and audio effects
- **UI/UX Design**: Custom graphics and user interface
- **Cross-platform Development**: CMake and portable C++

## License

This project is intended as a portfolio demonstration and learning resource.