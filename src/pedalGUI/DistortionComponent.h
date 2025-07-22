#pragma once

#include "PedalComponent.h"
#include "../pedalSoundEffects/Distortion.h"
#include "CustomToggleButton.h"
#include "CustomLookAndFeel.h"

class DistortionComponent : public PedalComponent
{
public:
    DistortionComponent()
        : PedalComponent("Distortion", std::make_unique<Distortion>(), "Distortion.png")
    {
        distortionPtr = dynamic_cast<Distortion*>(effectProcessor.get());

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
            if (distortionPtr)
                distortionPtr->setMix(volumeKnob.getValue());
        };
        addAndMakeVisible(volumeKnob);

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
            if (distortionPtr)
                distortionPtr->setGain(gainKnob.getValue());
        };
        addAndMakeVisible(gainKnob);

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
            if (distortionPtr)
                distortionPtr->setTone(toneKnob.getValue());
        };
        addAndMakeVisible(toneKnob);

        // === PEDAL BUTTON ===
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

        const int leftX = 73;
        const int topY = 20;

        const int rightX = leftX + 54;
        const int toneX = leftX + 27;
        const int toneY = topY + 14;

        volumeKnob.setBounds(leftX - knobSize / 2, topY - knobSize / 2, knobSize, knobSize);
        gainKnob.setBounds(rightX - knobSize / 2, topY - knobSize / 2, knobSize, knobSize);
        toneKnob.setBounds(toneX - toneSize / 2, toneY - toneSize / 2, toneSize, toneSize);

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
        obj->setProperty("volume", volumeKnob.getValue());
        obj->setProperty("gain", gainKnob.getValue());
        obj->setProperty("tone", toneKnob.getValue());
        return juce::var(obj);
    }
    
    void setKnobStates(const juce::var& states) override
    {
        if (auto* obj = states.getDynamicObject())
        {
            if (obj->hasProperty("volume"))
                volumeKnob.setValue(obj->getProperty("volume"), juce::dontSendNotification);
            if (obj->hasProperty("gain"))
                gainKnob.setValue(obj->getProperty("gain"), juce::dontSendNotification);
            if (obj->hasProperty("tone"))
                toneKnob.setValue(obj->getProperty("tone"), juce::dontSendNotification);
            
            // Update the effect processor
            if (distortionPtr)
            {
                distortionPtr->setMix(volumeKnob.getValue());
                distortionPtr->setGain(gainKnob.getValue());
                distortionPtr->setTone(toneKnob.getValue());
            }
        }
    }
    
    void updateToggleButton() override
    {
        pedalButton.setToggleState(isEnabled(), juce::dontSendNotification);
        pedalButton.repaint();
    }

private:
    juce::Slider volumeKnob, gainKnob, toneKnob;
    CustomToggleButton pedalButton;
    float level = 1.0f;
    Distortion* distortionPtr = nullptr;
    inline static CustomLookAndFeel customKnobLook;
};
