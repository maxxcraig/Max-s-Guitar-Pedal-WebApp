#pragma once
#include "BaseEffects.h"

class Overdrive : public BaseEffect {
public:
    Overdrive();

    void setGain(float newGain);
    float processSample(float inputSample) override;
    void setParameter(float value) override {
        setGain(value * 0.8f + 0.5f); // Scale parameter to reasonable range
    }

private:
    float gain = 2.0f;
    float mix = 0.8f;
    float outputLevel = 0.6f;
    float lastSample = 0.0f;
    float lastOutput = 0.0f;
};
