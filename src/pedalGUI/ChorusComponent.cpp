#include "ChorusComponent.h"
#include "CustomLookAndFeel.h"

static CustomLookAndFeel customKnobLook;

ChorusComponent::ChorusComponent()
    : PedalComponent("Chorus", std::make_unique<Chorus>(), "Chorus.png")
{
    chorusPtr = dynamic_cast<Chorus*>(effectProcessor.get());
    if (chorusPtr)
    {
        chorusPtr->setRate(0.8f);
        chorusPtr->setDepth(0.05f);
        chorusPtr->setCentreDelay(12.0f);  // <== this is critical!
        chorusPtr->setFeedback(0.0f);
        chorusPtr->setMix(0.25f);
    }


    // === RATE ===
    rateKnob.setLookAndFeel(&customKnobLook);
    rateKnob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    rateKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    rateKnob.setRange(0.1, 5.0, 0.01);
    rateKnob.setValue(2.55); // Middle of 0.1 to 5.0 range
    rateKnob.setRotaryParameters(juce::MathConstants<float>::pi * 0.7f,
                                  juce::MathConstants<float>::pi * 2.3f,
                                  true);
    rateKnob.onValueChange = [this]() {
        if (chorusPtr)
            chorusPtr->setRate(rateKnob.getValue());
    };
    addAndMakeVisible(rateKnob);
    rateKnob.setEnabled(true);

    // === DEPTH ===
    depthKnob.setLookAndFeel(&customKnobLook);
    depthKnob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    depthKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    depthKnob.setRange(0.0, 1.0, 0.01);
    depthKnob.setValue(0.5); // Middle of 0.0 to 1.0 range
    depthKnob.setRotaryParameters(juce::MathConstants<float>::pi * 0.7f,
                                   juce::MathConstants<float>::pi * 2.3f,
                                   true);
    depthKnob.onValueChange = [this]() {
        if (chorusPtr)
            chorusPtr->setDepth(depthKnob.getValue());
    };
    addAndMakeVisible(depthKnob);
    depthKnob.setEnabled(true);

    // === MIX ===
    mixKnob.setLookAndFeel(&customKnobLook);
    mixKnob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    mixKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    mixKnob.setRange(0.0, 1.0, 0.01);
    mixKnob.setValue(0.5); // Middle of 0.0 to 1.0 range
    mixKnob.setRotaryParameters(juce::MathConstants<float>::pi * 0.7f,
                                 juce::MathConstants<float>::pi * 2.3f,
                                 true);
    mixKnob.onValueChange = [this]() {
        if (chorusPtr)
            chorusPtr->setMix(mixKnob.getValue());
    };
    addAndMakeVisible(mixKnob);
    mixKnob.setEnabled(true);

    // === TOGGLE ===
    pedalButton.setClickingTogglesState(true);
    pedalButton.onClick = [this]() {
        setEnabled(pedalButton.getToggleState());
        pedalButton.repaint();
    };
    addAndMakeVisible(pedalButton);
}

void ChorusComponent::resized()
{
    const int knobSize = 38;
    const int mixSize = 32;

    const int rateCenterX = 73;
    const int rateCenterY = 20;

    const int depthCenterX = rateCenterX + 54;
    const int depthCenterY = rateCenterY;

    const int mixCenterX = rateCenterX + 27;
    const int mixCenterY = rateCenterY + 17;

    rateKnob.setBounds(rateCenterX - knobSize / 2, rateCenterY - knobSize / 2, knobSize, knobSize);
    depthKnob.setBounds(depthCenterX - knobSize / 2, depthCenterY - knobSize / 2, knobSize, knobSize);
    mixKnob.setBounds(mixCenterX - mixSize / 2, mixCenterY - mixSize / 2, mixSize, mixSize);

    pedalButton.setBounds(51, getHeight() - 61, 98, 52);
}


float ChorusComponent::processSample(float sample)
{
    if (effectProcessor && isEnabled())
        sample = effectProcessor->processSample(sample);
    return sample * level;
}

juce::var ChorusComponent::getKnobStates() const
{
    juce::DynamicObject::Ptr obj = new juce::DynamicObject();
    obj->setProperty("rate", rateKnob.getValue());
    obj->setProperty("depth", depthKnob.getValue());
    obj->setProperty("mix", mixKnob.getValue());
    return juce::var(obj);
}

void ChorusComponent::setKnobStates(const juce::var& states)
{
    if (auto* obj = states.getDynamicObject())
    {
        if (obj->hasProperty("rate"))
            rateKnob.setValue(obj->getProperty("rate"), juce::dontSendNotification);
        if (obj->hasProperty("depth"))
            depthKnob.setValue(obj->getProperty("depth"), juce::dontSendNotification);
        if (obj->hasProperty("mix"))
            mixKnob.setValue(obj->getProperty("mix"), juce::dontSendNotification);
        
        // Update the effect processor
        if (chorusPtr)
        {
            chorusPtr->setRate(rateKnob.getValue());
            chorusPtr->setDepth(depthKnob.getValue());
            chorusPtr->setMix(mixKnob.getValue());
        }
    }
}

void ChorusComponent::updateToggleButton()
{
    pedalButton.setToggleState(isEnabled(), juce::dontSendNotification);
    pedalButton.repaint();
}
