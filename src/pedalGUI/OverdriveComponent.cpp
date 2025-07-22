#include "OverdriveComponent.h"
#include "CustomLookAndFeel.h"

static CustomLookAndFeel customKnobLook;

OverdriveComponent::OverdriveComponent()
    : PedalComponent("Overdrive", std::make_unique<Overdrive>(), "overdrive.png")
{
    overdrivePtr = dynamic_cast<Overdrive*>(effectProcessor.get());

    // === LEVEL KNOB ===
    levelKnob.setLookAndFeel(&customKnobLook);
    levelKnob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    levelKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    levelKnob.setRange(0.0, 2.0, 0.01);
    levelKnob.setValue(1.0);
    levelKnob.setRotaryParameters(juce::MathConstants<float>::pi * 0.7f,
                                  juce::MathConstants<float>::pi * 2.3f,
                                  true);
    levelKnob.onValueChange = [this]() {
        level = levelKnob.getValue();
    };
    addAndMakeVisible(levelKnob);
    levelKnob.setEnabled(true);


    // === DRIVE KNOB ===
    driveKnob.setLookAndFeel(&customKnobLook);
    driveKnob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    driveKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    driveKnob.setRange(0.0, 10.0, 0.1);
    driveKnob.setValue(5.0);
    driveKnob.setRotaryParameters(juce::MathConstants<float>::pi * 0.7f,
                                  juce::MathConstants<float>::pi * 2.3f,
                                  true);
    driveKnob.onValueChange = [this]() {
        if (overdrivePtr)
            overdrivePtr->setGain(driveKnob.getValue());
    };
    addAndMakeVisible(driveKnob);
    driveKnob.setEnabled(true);


    // === TONE KNOB ===
    toneKnob.setLookAndFeel(&customKnobLook);
    toneKnob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    toneKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    toneKnob.setRange(0.0, 10.0, 0.1);
    toneKnob.setValue(5.0);
    toneKnob.setRotaryParameters(juce::MathConstants<float>::pi * 0.7f,
                                 juce::MathConstants<float>::pi * 2.3f,
                                 true);
    addAndMakeVisible(toneKnob);

    // === TOGGLE BUTTON ===
    pedalButton.setClickingTogglesState(true);
    addAndMakeVisible(pedalButton);
    toneKnob.setEnabled(true);

    pedalButton.onClick = [this]()
    {
        setEnabled(pedalButton.getToggleState());
        pedalButton.repaint();
    };



}

void OverdriveComponent::resized()
{
    const int knobSize = 38;
    const int toneSize = 32;

    const int levelCenterX = 73;
    const int levelCenterY = 20;

    const int driveCenterX = levelCenterX + 54;
    const int driveCenterY = levelCenterY;

    const int toneCenterX = levelCenterX + 27;
    const int toneCenterY = levelCenterY + 17;

    levelKnob.setBounds(levelCenterX - knobSize / 2, levelCenterY - knobSize / 2, knobSize, knobSize);
    driveKnob.setBounds(driveCenterX - knobSize / 2, driveCenterY - knobSize / 2, knobSize, knobSize);
    toneKnob.setBounds(toneCenterX - toneSize / 2, toneCenterY - toneSize / 2, toneSize, toneSize);

    // Position toggle at the bottom
    pedalButton.setBounds(51, getHeight() - 61, 98, 52); 
}

float OverdriveComponent::processSample(float sample)
{
    if (effectProcessor && isEnabled())
        sample = effectProcessor->processSample(sample);
    return sample * level;
}

juce::var OverdriveComponent::getKnobStates() const
{
    juce::DynamicObject::Ptr obj = new juce::DynamicObject();
    obj->setProperty("level", levelKnob.getValue());
    obj->setProperty("drive", driveKnob.getValue());
    obj->setProperty("tone", toneKnob.getValue());
    return juce::var(obj);
}

void OverdriveComponent::setKnobStates(const juce::var& states)
{
    if (auto* obj = states.getDynamicObject())
    {
        if (obj->hasProperty("level"))
            levelKnob.setValue(obj->getProperty("level"), juce::dontSendNotification);
        if (obj->hasProperty("drive"))
            driveKnob.setValue(obj->getProperty("drive"), juce::dontSendNotification);
        if (obj->hasProperty("tone"))
            toneKnob.setValue(obj->getProperty("tone"), juce::dontSendNotification);
        
        // Update the level variable
        level = levelKnob.getValue();
        
        // Update the effect processor
        if (overdrivePtr)
            overdrivePtr->setGain(driveKnob.getValue());
    }
}

void OverdriveComponent::updateToggleButton()
{
    pedalButton.setToggleState(isEnabled(), juce::dontSendNotification);
    pedalButton.repaint();
}
