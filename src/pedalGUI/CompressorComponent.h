#pragma once

#include "PedalComponent.h"
#include "../pedalSoundEffects/Compressor.h"
#include "CustomToggleButton.h"
#include "CustomLookAndFeel.h"

class CompressorComponent : public PedalComponent
{
public:
    CompressorComponent()
        : PedalComponent("Compressor", std::make_unique<Compressor>(), "Compressor.png")
    {
        compressorPtr = dynamic_cast<Compressor*>(effectProcessor.get());

        // === THRESHOLD KNOB ===
        thresholdKnob.setLookAndFeel(&customKnobLook);
        thresholdKnob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        thresholdKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        thresholdKnob.setRange(0.0, 10.0, 0.1);
        thresholdKnob.setValue(5.0);
        thresholdKnob.setRotaryParameters(juce::MathConstants<float>::pi * 0.7f,
                                         juce::MathConstants<float>::pi * 2.3f,
                                         true);
        thresholdKnob.onValueChange = [this]() {
            if (compressorPtr)
                compressorPtr->setParameter(thresholdKnob.getValue());
        };
        addAndMakeVisible(thresholdKnob);

        // === RATIO KNOB ===
        ratioKnob.setLookAndFeel(&customKnobLook);
        ratioKnob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        ratioKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        ratioKnob.setRange(1.0, 10.0, 0.1);
        ratioKnob.setValue(5.5);
        ratioKnob.setRotaryParameters(juce::MathConstants<float>::pi * 0.7f,
                                     juce::MathConstants<float>::pi * 2.3f,
                                     true);
        ratioKnob.onValueChange = [this]() {
            if (compressorPtr)
                compressorPtr->setRatio(ratioKnob.getValue());
        };
        addAndMakeVisible(ratioKnob);

        // === GAIN KNOB ===
        gainKnob.setLookAndFeel(&customKnobLook);
        gainKnob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        gainKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        gainKnob.setRange(0.5, 3.0, 0.1);
        gainKnob.setValue(1.75);
        gainKnob.setRotaryParameters(juce::MathConstants<float>::pi * 0.7f,
                                      juce::MathConstants<float>::pi * 2.3f,
                                      true);
        gainKnob.onValueChange = [this]() {
            if (compressorPtr)
                compressorPtr->setMakeupGain(gainKnob.getValue());
        };
        addAndMakeVisible(gainKnob);

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
    const int knobSize = 38;    // size for threshold & makeup
    const int ratioSize = 32;   // smaller knob for ratio

    const int thresholdCenterX = 73;
    const int thresholdCenterY = 20;

    const int gainCenterX = thresholdCenterX + 54;
    const int gainCenterY = thresholdCenterY;

    const int ratioCenterX = thresholdCenterX + 27;
    const int ratioCenterY = thresholdCenterY + 14;

    thresholdKnob.setBounds(thresholdCenterX - knobSize / 2, thresholdCenterY - knobSize / 2, knobSize, knobSize);
    gainKnob.setBounds(gainCenterX - knobSize / 2, gainCenterY - knobSize / 2, knobSize, knobSize);
    ratioKnob.setBounds(ratioCenterX - ratioSize / 2, ratioCenterY - ratioSize / 2, ratioSize, ratioSize);

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
        obj->setProperty("threshold", thresholdKnob.getValue());
        obj->setProperty("ratio", ratioKnob.getValue());
        obj->setProperty("gain", gainKnob.getValue());
        return juce::var(obj);
    }
    
    void setKnobStates(const juce::var& states) override
    {
        if (auto* obj = states.getDynamicObject())
        {
            if (obj->hasProperty("threshold"))
                thresholdKnob.setValue(obj->getProperty("threshold"), juce::dontSendNotification);
            if (obj->hasProperty("ratio"))
                ratioKnob.setValue(obj->getProperty("ratio"), juce::dontSendNotification);
            if (obj->hasProperty("gain"))
                gainKnob.setValue(obj->getProperty("gain"), juce::dontSendNotification);
            
            // Update the effect processor
            if (compressorPtr)
            {
                compressorPtr->setParameter(thresholdKnob.getValue());
                compressorPtr->setRatio(ratioKnob.getValue());
                compressorPtr->setMakeupGain(gainKnob.getValue());
            }
        }
    }
    
    void updateToggleButton() override
    {
        pedalButton.setToggleState(isEnabled(), juce::dontSendNotification);
        pedalButton.repaint();
    }

private:
    juce::Slider thresholdKnob, ratioKnob, gainKnob;
    CustomToggleButton pedalButton;
    float level = 1.0f;
    Compressor* compressorPtr = nullptr;
    inline static CustomLookAndFeel customKnobLook;
};