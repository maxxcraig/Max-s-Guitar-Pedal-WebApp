#pragma once
#include "BaseEffects.h"
#include <cmath>
#include <algorithm>

class Distortion : public BaseEffect {
public:
    float processSample(float sample) override {
        // Extreme multi-stage distortion with hard clipping
        float preGain = sample * gain;
        
        // First stage: Aggressive overdrive
        float stage1 = std::tanh(preGain * 1.2f);
        
        // Second stage: Hard clipping for extreme distortion
        float stage2 = stage1 * 2.0f;
        if (stage2 > 0.8f) stage2 = 0.8f + (stage2 - 0.8f) * 0.2f;
        else if (stage2 < -0.8f) stage2 = -0.8f + (stage2 + 0.8f) * 0.2f;
        
        // Third stage: Tube saturation
        float stage3 = std::tanh(stage2 * 1.5f) * 0.8f;
        
        // Aggressive tone control with bite
        float lowPass = 0.6f * stage3 + 0.4f * lastLowPass;
        lastLowPass = lowPass;
        
        float highPass = stage3 - 0.8f * lastHighPass;
        lastHighPass = stage3;
        
        // Emphasize high frequencies for more bite
        float toned = tone * (highPass * 1.3f) + (1.0f - tone) * lowPass;
        
        // Add some controlled chaos
        float distorted = toned * (1.0f + 0.1f * std::sin(toned * 50.0f));
        
        // Mix with dry signal
        return (1.0f - mix) * sample + mix * distorted * volume;
    }

    void setGain(float value) {
        gain = std::clamp(value, 0.5f, 6.0f);
    }

    void setTone(float value) {
        tone = std::clamp(value, 0.0f, 1.0f);
    }

    void setMix(float value) {
        volume = std::clamp(value, 0.1f, 0.8f);
    }

    void setParameter(float value) override {
        setGain(value * 0.8f + 1.5f);  // Scale to more extreme range
    }

    void setSampleRate(double rate) override {}

private:
    float gain = 3.5f;
    float tone = 0.7f;
    float volume = 0.5f;
    float mix = 0.95f;
    float lastLowPass = 0.0f;
    float lastHighPass = 0.0f;
};
