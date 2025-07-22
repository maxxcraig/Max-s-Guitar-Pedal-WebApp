cmake_minimum_required(VERSION 3.20)
project(GuitarPedalApp VERSION 0.1.0 LANGUAGES C CXX)

set(JUCE_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(JUCE_BUILD_EXTRAS OFF CACHE BOOL "" FORCE)
set(JUCE_BUILD_HELPER_TOOLS OFF CACHE BOOL "" FORCE)

add_subdirectory(external/JUCE)

include("${CMAKE_CURRENT_SOURCE_DIR}/external/JUCE/extras/Build/CMake/JUCEUtils.cmake")

juce_add_gui_app(GuitarPedalApp
    PRODUCT_NAME "GuitarPedalApp"
    MICROPHONE_PERMISSION_ENABLED TRUE
    MICROPHONE_PERMISSION_TEXT "This app needs access to your microphone to process guitar audio through the pedal effects."
)

juce_generate_juce_header(GuitarPedalApp)

target_link_libraries(GuitarPedalApp
    PRIVATE
        juce::juce_audio_utils
        juce::juce_audio_basics
        juce::juce_audio_devices
        juce::juce_audio_formats
        juce::juce_audio_processors
        juce::juce_core
        juce::juce_data_structures
        juce::juce_events
        juce::juce_graphics
        juce::juce_gui_basics
        juce::juce_gui_extra
        juce::juce_dsp
)

target_sources(GuitarPedalApp
    PRIVATE
        src/main.cpp
        src/pedalGUI/PedalComponent.cpp 
        src/ScreenRouter.cpp
        src/pedalSoundEffects/Overdrive.cpp
        src/pedalSoundEffects/CustomReverb.cpp
        src/pedalGUI/OverdriveComponent.cpp  
        src/pedalGUI/ChorusComponent.cpp  
        src/screens/PedalBoardScreen.cpp
        src/screens/SavedBoardsScreen.cpp
        src/auth/SupabaseClient.cpp
        src/auth/AuthManager.cpp
)
    
        


target_include_directories(GuitarPedalApp
    PRIVATE
        src
        src/pedalGUI
        src/pedalSoundEffects
        src/screens
        src/auth
)

target_compile_features(GuitarPedalApp
    PRIVATE
        cxx_std_17
)

# Platform-specific resource copying
if(APPLE)
    file(GLOB RESOURCE_FILES "${CMAKE_SOURCE_DIR}/resources/*")

    foreach(RESOURCE_FILE ${RESOURCE_FILES})
        get_filename_component(FILENAME ${RESOURCE_FILE} NAME)
        add_custom_command(TARGET GuitarPedalApp POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy
            ${RESOURCE_FILE}
            "$<TARGET_BUNDLE_DIR:GuitarPedalApp>/Contents/Resources/${FILENAME}"
        )
    endforeach()

    # Copy .env file to app bundle if it exists
    if(EXISTS "${CMAKE_SOURCE_DIR}/.env")
        add_custom_command(TARGET GuitarPedalApp POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy
            "${CMAKE_SOURCE_DIR}/.env"
            "$<TARGET_BUNDLE_DIR:GuitarPedalApp>/Contents/Resources/.env"
        )
    endif()
elseif(WIN32)
    file(GLOB RESOURCE_FILES "${CMAKE_SOURCE_DIR}/resources/*")

    foreach(RESOURCE_FILE ${RESOURCE_FILES})
        get_filename_component(FILENAME ${RESOURCE_FILE} NAME)
        add_custom_command(TARGET GuitarPedalApp POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy
            ${RESOURCE_FILE}
            "$<TARGET_FILE_DIR:GuitarPedalApp>/${FILENAME}"
        )
    endforeach()

    # Copy .env file to executable directory if it exists
    if(EXISTS "${CMAKE_SOURCE_DIR}/.env")
        add_custom_command(TARGET GuitarPedalApp POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy
            "${CMAKE_SOURCE_DIR}/.env"
            "$<TARGET_FILE_DIR:GuitarPedalApp>/.env"
        )
    endif()
endif()


