#pragma once

#include "PedalComponent.h"
#include "../pedalSoundEffects/Octave.h"
#include "CustomToggleButton.h"
#include "CustomLookAndFeel.h"

class OctaveComponent : public PedalComponent
{
public:
    OctaveComponent()
        : PedalComponent("Octave", std::make_unique<Octave>(), "Octave.png")
    {
        octavePtr = dynamic_cast<Octave*>(effectProcessor.get());

        // === BLEND KNOB ===
        blendKnob.setLookAndFeel(&customKnobLook);
        blendKnob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        blendKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        blendKnob.setRange(0.0, 10.0, 0.1);
        blendKnob.setValue(5.0);
        blendKnob.setRotaryParameters(juce::MathConstants<float>::pi * 0.7f,
                                     juce::MathConstants<float>::pi * 2.3f,
                                     true);
        blendKnob.onValueChange = [this]() {
            if (octavePtr) {
                // Map 0-10 knob to 0-1.0 blend range
                float mappedBlend = blendKnob.getValue() / 10.0f;
                octavePtr->setBlend(mappedBlend);
            }
        };
        addAndMakeVisible(blendKnob);

        // === TONE KNOB ===
        toneKnob.setLookAndFeel(&customKnobLook);
        toneKnob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        toneKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        toneKnob.setRange(0.0, 1.0, 0.01);
        toneKnob.setValue(0.5);
        toneKnob.setRotaryParameters(juce::MathConstants<float>::pi * 0.7f,
                                     juce::MathConstants<float>::pi * 2.3f,
                                     true);
        toneKnob.onValueChange = [this]() {
            if (octavePtr) {
                octavePtr->setTone(toneKnob.getValue());
            }
        };
        addAndMakeVisible(toneKnob);

        // === OCTAVE LEVEL KNOB ===
        octaveLevelKnob.setLookAndFeel(&customKnobLook);
        octaveLevelKnob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        octaveLevelKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        octaveLevelKnob.setRange(0.0, 2.0, 0.1);
        octaveLevelKnob.setValue(1.0);
        octaveLevelKnob.setRotaryParameters(juce::MathConstants<float>::pi * 0.7f,
                                           juce::MathConstants<float>::pi * 2.3f,
                                           true);
        octaveLevelKnob.onValueChange = [this]() {
            if (octavePtr) {
                octavePtr->setOctaveLevel(octaveLevelKnob.getValue());
            }
        };
        addAndMakeVisible(octaveLevelKnob);

        // === BYPASS BUTTON ===
        pedalButton.setClickingTogglesState(true);
        pedalButton.onClick = [this]() {
            setEnabled(pedalButton.getToggleState());
            pedalButton.repaint();
        };
        addAndMakeVisible(pedalButton);
    }

    void resized() override
    {
        const int knobSize = 38;    // size for blend & octave level
        const int toneSize = 32;    // smaller knob for tone

        const int blendCenterX = 73;
        const int blendCenterY = 20;

        const int octaveCenterX = blendCenterX + 54;
        const int octaveCenterY = blendCenterY;

        const int toneCenterX = blendCenterX + 27;
        const int toneCenterY = blendCenterY + 17;

        blendKnob.setBounds(blendCenterX - knobSize / 2, blendCenterY - knobSize / 2, knobSize, knobSize);
        octaveLevelKnob.setBounds(octaveCenterX - knobSize / 2, octaveCenterY - knobSize / 2, knobSize, knobSize);
        toneKnob.setBounds(toneCenterX - toneSize / 2, toneCenterY - toneSize / 2, toneSize, toneSize);

        pedalButton.setBounds(51, getHeight() - 61, 98, 52);
    }

    float processSample(float sample)
    {
        if (effectProcessor && isEnabled())
            sample = effectProcessor->processSample(sample);
        return sample * level;
    }
    
    // Save/load knob states
    juce::var getKnobStates() const override
    {
        juce::DynamicObject::Ptr obj = new juce::DynamicObject();
        obj->setProperty("blend", blendKnob.getValue());
        obj->setProperty("tone", toneKnob.getValue());
        obj->setProperty("octave", octaveLevelKnob.getValue());
        return juce::var(obj);
    }
    
    void setKnobStates(const juce::var& states) override
    {
        if (auto* obj = states.getDynamicObject())
        {
            if (obj->hasProperty("blend"))
                blendKnob.setValue(obj->getProperty("blend"), juce::dontSendNotification);
            if (obj->hasProperty("tone"))
                toneKnob.setValue(obj->getProperty("tone"), juce::dontSendNotification);
            if (obj->hasProperty("octave"))
                octaveLevelKnob.setValue(obj->getProperty("octave"), juce::dontSendNotification);
            
            // Update the effect processor
            if (octavePtr)
            {
                float mappedBlend = blendKnob.getValue() / 10.0f;
                octavePtr->setBlend(mappedBlend);
                octavePtr->setTone(toneKnob.getValue());
                octavePtr->setOctaveLevel(octaveLevelKnob.getValue());
            }
        }
    }
    
    void updateToggleButton() override
    {
        pedalButton.setToggleState(isEnabled(), juce::dontSendNotification);
        pedalButton.repaint();
    }

private:
    juce::Slider blendKnob, toneKnob, octaveLevelKnob;
    CustomToggleButton pedalButton;
    float level = 1.0f;
    Octave* octavePtr = nullptr;
    inline static CustomLookAndFeel customKnobLook;
};