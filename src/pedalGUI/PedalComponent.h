#pragma once

#include <JuceHeader.h>
#include "BaseEffects.h"
#include <vector>

// Helper function to get resources from app bundle
inline juce::File getAppBundleResource(const juce::String& resourceName) {
    auto appFile = juce::File::getSpecialLocation(juce::File::currentApplicationFile);
    return appFile.getChildFile("Contents/Resources/" + resourceName);
}

class PedalComponent : public juce::Component
{
public:
    PedalComponent(const juce::String& name, std::unique_ptr<BaseEffect> effect, const juce::String& imagePath);
    virtual ~PedalComponent();

    void paint(juce::Graphics& g) override;
    virtual void resized() override;

    float processSample(float sample);
    void setSampleRate(double rate);

    void setOriginalBounds(juce::Rectangle<int> bounds);
    juce::Rectangle<int> getOriginalBounds() const;

    static void registerPedal(PedalComponent* pedal);
    static void unregisterPedal(PedalComponent* pedal);
    static void clearAllPedals();
    static std::vector<PedalComponent*> getRegisteredPedals();

    void mouseDown(const juce::MouseEvent& event) override;
    void mouseDrag(const juce::MouseEvent& event) override;
    void mouseUp(const juce::MouseEvent& event) override;

    const juce::String& getName() const { return pedalName; }
    bool isEnabled() const { return isActive; }
    virtual void setEnabled(bool enabled) { isActive = enabled; updateToggleButton(); }
    
    // Virtual method for pedal subclasses to update their toggle button
    virtual void updateToggleButton() {}
    
    // Virtual methods for saving/loading knob states
    virtual juce::var getKnobStates() const { return juce::var(); }
    virtual void setKnobStates(const juce::var& states) { (void)states; }

protected:
    juce::String pedalName;
    juce::String imagePath;
    std::unique_ptr<BaseEffect> effectProcessor;
    juce::Image pedalImage;
    bool isActive = false;

private:
    juce::Point<int> dragOffset;
    juce::Rectangle<int> originalBounds;

    static std::vector<PedalComponent*> allPedals;
};
