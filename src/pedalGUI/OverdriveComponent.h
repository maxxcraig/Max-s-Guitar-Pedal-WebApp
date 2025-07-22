#pragma once

#include "PedalComponent.h"
#include "../pedalSoundEffects/Overdrive.h"
#include "CustomToggleButton.h"



class OverdriveComponent : public PedalComponent
{
public:
    OverdriveComponent();
    void resized() override;
    float processSample(float sample);
    
    // Save/load knob states
    juce::var getKnobStates() const override;
    void setKnobStates(const juce::var& states) override;
    
    // Update toggle button to match enabled state
    void updateToggleButton() override;

private:
    juce::Slider levelKnob, driveKnob, toneKnob;
    float level = 1.0f;
    Overdrive* overdrivePtr = nullptr;
    CustomToggleButton pedalButton;
};
