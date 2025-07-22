#pragma once

#include "BaseEffects.h"
#include <vector>
#include <cmath>
#include <algorithm>

class CustomReverb : public BaseEffect {
public:
    CustomReverb();
    float processSample(float input) override;
    void setParameter(float value) override;
    void setSampleRate(double rate) override;

    void setMix(float value);
    void setDecay(float value);
    void setTone(float value);

private:
    void initializeDelayLines();
    
    struct DelayLine {
        std::vector<float> buffer;
        int writeIndex = 0;
        int delaySamples = 0;
        float feedback = 0.0f;
        
        float process(float input) {
            if (buffer.empty()) return input;
            
            float output = buffer[writeIndex];
            buffer[writeIndex] = input + output * feedback;
            writeIndex = (writeIndex + 1) % buffer.size();
            return output;
        }
    };
    
    DelayLine earlyReflections[6];
    DelayLine lateReverb[4];
    
    float mix = 0.5f;
    float decay = 0.6f;
    float tone = 0.5f;
    float diffusion = 0.7f;
    double sampleRate = 44100.0;
    
    float lowpassState = 0.0f;
    float allpassStates[4] = {0.0f};
};
