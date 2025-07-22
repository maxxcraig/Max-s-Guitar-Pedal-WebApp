#pragma once

#include "PedalComponent.h"
#include "../pedalSoundEffects/Chorus.h"
#include "CustomToggleButton.h"

class ChorusComponent : public PedalComponent
{
public:
    ChorusComponent();
    void resized() override;
    float processSample(float sample);
    
    // Save/load knob states
    juce::var getKnobStates() const override;
    void setKnobStates(const juce::var& states) override;
    
    // Update toggle button to match enabled state
    void updateToggleButton() override;

private:
    juce::Slider rateKnob, depthKnob, mixKnob;
    float level = 1.0f;
    Chorus* chorusPtr = nullptr;
    CustomToggleButton pedalButton;
};
