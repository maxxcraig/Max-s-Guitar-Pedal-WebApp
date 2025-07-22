#pragma once

#include "PedalComponent.h"
#include "../pedalSoundEffects/BluesDriver.h"
#include "CustomToggleButton.h"
#include "CustomLookAndFeel.h"

class BluesDriverComponent : public PedalComponent
{
public:
    BluesDriverComponent()
        : PedalComponent("Blues Driver", std::make_unique<BluesDriver>(), "BluesDriver.png")
    {
        bluesPtr = dynamic_cast<BluesDriver*>(effectProcessor.get());

        // === GAIN KNOB ===
        gainKnob.setLookAndFeel(&customKnobLook);
        gainKnob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        gainKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        gainKnob.setRange(0.0, 10.0, 0.1);
        gainKnob.setValue(5.0);
        gainKnob.setRotaryParameters(juce::MathConstants<float>::pi * 0.7f,
                                     juce::MathConstants<float>::pi * 2.3f,
                                     true);
        gainKnob.onValueChange = [this]() {
            if (bluesPtr)
                bluesPtr->setGain(gainKnob.getValue());
        };
        addAndMakeVisible(gainKnob);

        // === VOLUME KNOB ===
        volumeKnob.setLookAndFeel(&customKnobLook);
        volumeKnob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        volumeKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        volumeKnob.setRange(0.0, 1.0, 0.01);
        volumeKnob.setValue(0.5);
        volumeKnob.setRotaryParameters(juce::MathConstants<float>::pi * 0.7f,
                                       juce::MathConstants<float>::pi * 2.3f,
                                       true);
        volumeKnob.onValueChange = [this]() {
            if (bluesPtr)
                bluesPtr->setMix(volumeKnob.getValue());
        };
        addAndMakeVisible(volumeKnob);

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
            if (bluesPtr)
                bluesPtr->setTone(toneKnob.getValue());
        };
        addAndMakeVisible(toneKnob);

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
        const int knobSize = 38;
        const int toneSize = 32;

        const int gainCenterX = 73;
        const int gainCenterY = 20;

        const int volumeCenterX = gainCenterX + 54;
        const int volumeCenterY = gainCenterY;

        const int toneCenterX = gainCenterX + 27;
        const int toneCenterY = gainCenterY + 14;

        gainKnob.setBounds(gainCenterX - knobSize / 2, gainCenterY - knobSize / 2, knobSize, knobSize);
        volumeKnob.setBounds(volumeCenterX - knobSize / 2, volumeCenterY - knobSize / 2, knobSize, knobSize);
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
        obj->setProperty("gain", gainKnob.getValue());
        obj->setProperty("volume", volumeKnob.getValue());
        obj->setProperty("tone", toneKnob.getValue());
        return juce::var(obj);
    }
    
    void setKnobStates(const juce::var& states) override
    {
        if (auto* obj = states.getDynamicObject())
        {
            if (obj->hasProperty("gain"))
                gainKnob.setValue(obj->getProperty("gain"), juce::dontSendNotification);
            if (obj->hasProperty("volume"))
                volumeKnob.setValue(obj->getProperty("volume"), juce::dontSendNotification);
            if (obj->hasProperty("tone"))
                toneKnob.setValue(obj->getProperty("tone"), juce::dontSendNotification);
            
            // Update the effect processor
            if (bluesPtr)
            {
                bluesPtr->setGain(gainKnob.getValue());
                bluesPtr->setMix(volumeKnob.getValue());
                bluesPtr->setTone(toneKnob.getValue());
            }
        }
    }
    
    void updateToggleButton() override
    {
        pedalButton.setToggleState(isEnabled(), juce::dontSendNotification);
        pedalButton.repaint();
    }

private:
    juce::Slider gainKnob, volumeKnob, toneKnob;
    CustomToggleButton pedalButton;
    float level = 1.0f;
    BluesDriver* bluesPtr = nullptr;
    inline static CustomLookAndFeel customKnobLook;
};
