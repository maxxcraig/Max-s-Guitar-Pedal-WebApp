#pragma once
#include "BaseEffects.h"
#include <cmath>

class Tremolo : public BaseEffect {
public:
    float processSample(float sample) override {
        float mod = (std::sin(phase) + 1.0f) / 2.0f;
        phase += rate * 2.0f * juce::MathConstants<float>::pi / sampleRate;
        if (phase >= juce::MathConstants<float>::twoPi)
            phase -= juce::MathConstants<float>::twoPi;

        float tremSample = sample * ((1.0f - depth) + mod * depth);
        return (1.0f - mix) * sample + mix * tremSample * volume;
    }

    void setDepth(float d) { depth = d; }
    void setMix(float m) { mix = m; }
    void setVolume(float v) { volume = v; }
    void setRate(float r) { rate = r; }

    void setParameter(float value) override {
        depth = 0.4f + value / 15.0f; // Start at 0.4 instead of 0 for more baseline effect
    }

    void setSampleRate(double rateIn) override {
        sampleRate = rateIn;
    }

private:
    float rate = 5.0f;      // Tremolo speed in Hz (optional to expose in UI)
    float depth = 0.8f;     // How deep the tremolo modulates amplitude - increased for more prominence
    float mix = 0.75f;      // Wet/dry mix - increased for more effect
    float volume = 1.0f;    // Output volume
    float phase = 0.0f;
    double sampleRate = 44100.0;
};
