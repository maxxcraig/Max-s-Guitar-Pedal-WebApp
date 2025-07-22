#pragma once

#include "PedalComponent.h"
#include "../pedalSoundEffects/CustomReverb.h"
#include "CustomToggleButton.h"
#include "CustomLookAndFeel.h"

class ReverbComponent : public PedalComponent
{
public:
    ReverbComponent()
        : PedalComponent("Reverb", std::make_unique<CustomReverb>(), "Reverb.png")
    {
        reverbPtr = dynamic_cast<CustomReverb*>(effectProcessor.get());

        // === DECAY KNOB ===
        decayKnob.setLookAndFeel(&customKnobLook);
        decayKnob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        decayKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        decayKnob.setRange(0.0, 1.0, 0.01);
        decayKnob.setValue(0.5);
        decayKnob.setRotaryParameters(juce::MathConstants<float>::pi * 0.7f,
                                       juce::MathConstants<float>::pi * 2.3f,
                                       true);
        decayKnob.onValueChange = [this]() {
            if (reverbPtr)
                reverbPtr->setDecay(decayKnob.getValue());
        };
        addAndMakeVisible(decayKnob);

        // === MIX KNOB ===
        mixKnob.setLookAndFeel(&customKnobLook);
        mixKnob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        mixKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        mixKnob.setRange(0.0, 1.0, 0.01);
        mixKnob.setValue(0.5);
        mixKnob.setRotaryParameters(juce::MathConstants<float>::pi * 0.7f,
                                     juce::MathConstants<float>::pi * 2.3f,
                                     true);
        mixKnob.onValueChange = [this]() {
            if (reverbPtr)
                reverbPtr->setMix(mixKnob.getValue());
        };
        addAndMakeVisible(mixKnob);

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
            if (reverbPtr)
                reverbPtr->setTone(toneKnob.getValue());
        };
        addAndMakeVisible(toneKnob);

        // === TOGGLE BUTTON ===
        pedalButton.setClickingTogglesState(true);
        pedalButton.onClick = [this]() {
            setEnabled(pedalButton.getToggleState());
            pedalButton.repaint();
        };
        addAndMakeVisible(pedalButton);
    }

    void resized() override
    {
        const int knobSize = 38;
        const int toneSize = 32;

        const int decayCenterX = 73;
        const int decayCenterY = 20;

        const int mixCenterX = decayCenterX + 54;
        const int mixCenterY = decayCenterY;

        const int toneCenterX = decayCenterX + 27;
        const int toneCenterY = decayCenterY + 14;

        decayKnob.setBounds(decayCenterX - knobSize / 2, decayCenterY - knobSize / 2, knobSize, knobSize);
        mixKnob.setBounds(mixCenterX - knobSize / 2, mixCenterY - knobSize / 2, knobSize, knobSize);
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
        obj->setProperty("decay", decayKnob.getValue());
        obj->setProperty("mix", mixKnob.getValue());
        obj->setProperty("tone", toneKnob.getValue());
        return juce::var(obj);
    }
    
    void setKnobStates(const juce::var& states) override
    {
        if (auto* obj = states.getDynamicObject())
        {
            if (obj->hasProperty("decay"))
                decayKnob.setValue(obj->getProperty("decay"), juce::dontSendNotification);
            if (obj->hasProperty("mix"))
                mixKnob.setValue(obj->getProperty("mix"), juce::dontSendNotification);
            if (obj->hasProperty("tone"))
                toneKnob.setValue(obj->getProperty("tone"), juce::dontSendNotification);
            
            // Update the effect processor
            if (reverbPtr)
            {
                reverbPtr->setDecay(decayKnob.getValue());
                reverbPtr->setMix(mixKnob.getValue());
                reverbPtr->setTone(toneKnob.getValue());
            }
        }
    }
    
    void updateToggleButton() override
    {
        pedalButton.setToggleState(isEnabled(), juce::dontSendNotification);
        pedalButton.repaint();
    }

private:
    juce::Slider decayKnob, mixKnob, toneKnob;
    CustomToggleButton pedalButton;
    float level = 1.0f;
    CustomReverb* reverbPtr = nullptr;
    inline static CustomLookAndFeel customKnobLook;
};
