#pragma once

#include "PedalComponent.h"
#include "../pedalSoundEffects/Tremolo.h"
#include "CustomToggleButton.h"
#include "CustomLookAndFeel.h"

class TremoloComponent : public PedalComponent
{
public:
    TremoloComponent()
        : PedalComponent("Tremolo", std::make_unique<Tremolo>(), "Tremolo.png")
    {
        tremoloPtr = dynamic_cast<Tremolo*>(effectProcessor.get());

        // === DEPTH KNOB ===
        depthKnob.setLookAndFeel(&customKnobLook);
        depthKnob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        depthKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        depthKnob.setRange(0.0, 1.0, 0.01);
        depthKnob.setValue(0.5);
        depthKnob.setRotaryParameters(juce::MathConstants<float>::pi * 0.7f,
                                       juce::MathConstants<float>::pi * 2.3f,
                                       true);
        depthKnob.onValueChange = [this]() {
            if (tremoloPtr)
                tremoloPtr->setDepth(depthKnob.getValue());
        };
        addAndMakeVisible(depthKnob);

        // === VOLUME KNOB ===
        volumeKnob.setLookAndFeel(&customKnobLook);
        volumeKnob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        volumeKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        volumeKnob.setRange(0.0, 2.0, 0.01);
        volumeKnob.setValue(1.0);
        volumeKnob.setRotaryParameters(juce::MathConstants<float>::pi * 0.7f,
                                        juce::MathConstants<float>::pi * 2.3f,
                                        true);
        volumeKnob.onValueChange = [this]() {
            if (tremoloPtr)
                tremoloPtr->setVolume(volumeKnob.getValue());
        };
        addAndMakeVisible(volumeKnob);

        // === rate KNOB ===
        rateKnob.setLookAndFeel(&customKnobLook);
        rateKnob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        rateKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        rateKnob.setRange(0.0, 1.0, 0.01);
        rateKnob.setValue(0.5);
        rateKnob.setRotaryParameters(juce::MathConstants<float>::pi * 0.7f,
                                     juce::MathConstants<float>::pi * 2.3f,
                                     true);
        rateKnob.onValueChange = [this]() {
            if (tremoloPtr)
                tremoloPtr->setMix(rateKnob.getValue());
        };
        addAndMakeVisible(rateKnob);

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
        const int rateSize = 32;

        const int depthCenterX = 73;
        const int depthCenterY = 20;

        const int volumeCenterX = depthCenterX + 54;
        const int volumeCenterY = depthCenterY;

        const int rateCenterX = depthCenterX + 27;
        const int rateCenterY = depthCenterY + 14;

        depthKnob.setBounds(depthCenterX - knobSize / 2, depthCenterY - knobSize / 2, knobSize, knobSize);
        volumeKnob.setBounds(volumeCenterX - knobSize / 2, volumeCenterY - knobSize / 2, knobSize, knobSize);
        rateKnob.setBounds(rateCenterX - rateSize / 2, rateCenterY - rateSize / 2, rateSize, rateSize);

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
        obj->setProperty("depth", depthKnob.getValue());
        obj->setProperty("volume", volumeKnob.getValue());
        obj->setProperty("rate", rateKnob.getValue());
        return juce::var(obj);
    }
    
    void setKnobStates(const juce::var& states) override
    {
        if (auto* obj = states.getDynamicObject())
        {
            if (obj->hasProperty("depth"))
                depthKnob.setValue(obj->getProperty("depth"), juce::dontSendNotification);
            if (obj->hasProperty("volume"))
                volumeKnob.setValue(obj->getProperty("volume"), juce::dontSendNotification);
            if (obj->hasProperty("rate"))
                rateKnob.setValue(obj->getProperty("rate"), juce::dontSendNotification);
            
            // Update the effect processor
            if (tremoloPtr)
            {
                tremoloPtr->setDepth(depthKnob.getValue());
                tremoloPtr->setVolume(volumeKnob.getValue());
                tremoloPtr->setMix(rateKnob.getValue());
            }
        }
    }
    
    void updateToggleButton() override
    {
        pedalButton.setToggleState(isEnabled(), juce::dontSendNotification);
        pedalButton.repaint();
    }

private:
    juce::Slider depthKnob, volumeKnob, rateKnob;
    CustomToggleButton pedalButton;
    float level = 1.0f;
    Tremolo* tremoloPtr = nullptr;
    inline static CustomLookAndFeel customKnobLook;
};
