#pragma once

#include <JuceHeader.h>
#include "pedalGUI/PedalComponent.h"
#include "auth/SupabaseClient.h"
#include "auth/AuthManager.h"
#include "pedalGUI/OverdriveComponent.h"
#include "pedalGUI/ChorusComponent.h"
#include "pedalGUI/CompressorComponent.h"
#include "pedalGUI/TremoloComponent.h"
#include "pedalGUI/ReverbComponent.h"
#include "pedalGUI/OctaveComponent.h"
#include "pedalGUI/BluesDriverComponent.h"
#include "pedalGUI/DistortionComponent.h"

class PedalBoardScreen : public juce::AudioAppComponent {
public:
    PedalBoardScreen();
    ~PedalBoardScreen() override;

    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& info) override;
    void releaseResources() override;
    void paint(Graphics& g) override;
    void resized() override;
    PedalBoardScreen(std::function<void()> goHome);
    void loadBoardData(const juce::var& boardVar);
    PedalComponent* getPedalByName(const juce::String& name);

private:
    float softLimit(float sample);
    double currentSampleRate = 44100.0;
    juce::Image backgroundImage;

    std::unique_ptr<OverdriveComponent> overdrivePedal;
    std::unique_ptr<DistortionComponent> distortionPedal;
    std::unique_ptr<ReverbComponent> reverbPedal;
    std::unique_ptr<ChorusComponent> chorusPedal;
    std::unique_ptr<BluesDriverComponent> bluesDriverPedal;
    std::unique_ptr<CompressorComponent> delayPedal;
    std::unique_ptr<TremoloComponent> tremoloPedal;
    std::unique_ptr<OctaveComponent> phaserPedal;
    juce::TextButton backButton { "← Home" };
    juce::TextButton saveButton { "Save Board" };
    juce::TextButton bypassButton { "Bypass All" };
    juce::Slider inputGainSlider;
    juce::Label inputGainLabel { "Input Gain", "Input Gain" };
    
    // Cloud storage
    SupabaseClient supabaseClient;
    
    bool masterBypass = false;
    float inputGain = 0.3f;
    
    std::function<void()> onBackToHome;
};
