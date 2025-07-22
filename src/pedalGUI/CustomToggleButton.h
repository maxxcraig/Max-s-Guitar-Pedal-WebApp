#pragma once
#include <JuceHeader.h>

class CustomToggleButton : public juce::ToggleButton
{
public:
    void paintButton(juce::Graphics& g, bool isMouseOver, bool isButtonDown) override
    {
        auto bounds = getLocalBounds().toFloat();
        g.setColour(getToggleState() ? juce::Colours::white : juce::Colours::black);
        g.fillRect(bounds);
    }
};
