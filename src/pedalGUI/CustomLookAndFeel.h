#pragma once
#include <JuceHeader.h>

class CustomLookAndFeel : public juce::LookAndFeel_V4
{
public:
    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                          float sliderPosProportional, float rotaryStartAngle,
                          float rotaryEndAngle, juce::Slider& slider) override
    {
        auto bounds = juce::Rectangle<float>(x, y, width, height).reduced(4.0f);

        auto radius = std::min(bounds.getWidth(), bounds.getHeight()) / 2.0f;
        auto center = bounds.getCentre();
        auto angle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);

        // Draw black background arc
        g.setColour(juce::Colours::black);
        g.fillEllipse(bounds);

        // Shrink and draw blue indicator
        float indicatorRadius = radius * 0.2f;  // <<-- this controls the blue dot size
        juce::Point<float> thumbPoint(center.getX() + std::cos(angle) * (radius - 5.0f),
                                      center.getY() + std::sin(angle) * (radius - 5.0f));

        g.setColour(juce::Colours::white);
        g.fillEllipse(thumbPoint.x - indicatorRadius, thumbPoint.y - indicatorRadius,
                      indicatorRadius * 2.0f, indicatorRadius * 2.0f);
    }
};
