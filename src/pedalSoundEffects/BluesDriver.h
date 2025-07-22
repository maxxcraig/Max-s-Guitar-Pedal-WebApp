#pragma once
#include "BaseEffects.h"
#include <cmath>
#include <algorithm>

class BluesDriver : public BaseEffect {
public:
    float processSample(float sample) override {
        // Pre-emphasis for blues character
        float preEmphasized = sample + 0.4f * (sample - lastInput);
        lastInput = sample;
        
        // Blues-style asymmetric clipping with tube warmth
        float driven = preEmphasized * gain;
        
        // Asymmetric saturation (more compression on positive cycles like real tubes)
        float processed;
        if (driven > 0.0f) {
            processed = std::tanh(driven * 0.7f) * 0.85f;
        } else {
            processed = std::tanh(driven * 1.3f) * 0.75f;
        }
        
        // Add second harmonic for blues warmth
        float harmonic = processed * processed * 0.15f;
        processed += harmonic;
        
        // Blues-style tone stack (mid-scooped with controllable presence)
        float bass = processed * (1.0f - tone * 0.3f);
        float mid = processed * 0.7f;
        float treble = processed * tone * 1.2f;
        float toned = bass + mid + treble;
        
        // Tube-style compression with knee
        float compressed = toned / (1.0f + std::abs(toned) * 0.4f);
        
        // Add subtle tube-like even harmonics
        float enhanced = compressed * (1.0f + 0.05f * compressed * compressed);
        
        // Mix with dry signal
        return (1.0f - mix) * sample + mix * enhanced * volume;
    }

    void setGain(float value) {
        gain = std::clamp(value, 0.5f, 4.0f);
    }

    void setTone(float value) {
        tone = std::clamp(value, 0.0f, 1.0f);
    }

    void setMix(float value) {
        volume = std::clamp(value, 0.1f, 0.7f);
    }

    void setParameter(float value) override {
        setGain(value * 0.4f + 1.2f);  // Scale to moderate blues range
    }

    void setSampleRate(double rate) override {}

private:
    float gain = 2.2f;
    float tone = 0.65f;
    float volume = 0.6f;
    float mix = 0.85f;
    float lastInput = 0.0f;
};
