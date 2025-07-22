#pragma once
#include "BaseEffects.h"
#include <vector>
#include <cmath>
#include <algorithm>

// Define M_PI for Windows compatibility
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

class Chorus : public BaseEffect {
public:
    Chorus() {
        setSampleRate(44100.0);
    }

    float processSample(float input) override {
        if (delayBuffer.empty()) return input;

        // Clean up input to prevent fuzz
        input = std::tanh(input * 0.95f);

        float lfoValue = std::sin(lfoPhase) * depth;
        lfoPhase += rate * 2.0f * M_PI / sampleRate;
        if (lfoPhase > 2.0f * M_PI) lfoPhase -= 2.0f * M_PI;

        float delayTime = centreDelay + lfoValue;
        int delaySamples = static_cast<int>(delayTime * sampleRate / 1000.0f);
        delaySamples = std::clamp(delaySamples, 1, static_cast<int>(delayBuffer.size()) - 1);

        int readIndex = writeIndex - delaySamples;
        if (readIndex < 0) readIndex += delayBuffer.size();

        float delayedSample = delayBuffer[readIndex];
        
        // Reduce feedback to eliminate fuzz and apply soft clipping
        float feedbackSample = input * 0.8f + delayedSample * (feedback * 0.5f);
        feedbackSample = std::tanh(feedbackSample * 0.9f);
        delayBuffer[writeIndex] = feedbackSample;
        
        writeIndex = (writeIndex + 1) % delayBuffer.size();

        // Clean mix with slight attenuation to prevent artifacts
        float wetSignal = delayedSample * 0.85f;
        return std::clamp((1.0f - mix) * input + mix * wetSignal, -1.0f, 1.0f);
    }

    void setSampleRate(double rate) override {
        sampleRate = rate;
        int bufferSize = static_cast<int>(rate * maxDelayMs / 1000.0f) + 1;
        delayBuffer.assign(bufferSize, 0.0f);
        writeIndex = 0;
        lfoPhase = 0.0f;
    }

    void setRate(float Hz) { rate = std::clamp(Hz, 0.1f, 5.0f); }
    void setDepth(float d) { depth = std::clamp(d, 0.0f, 10.0f); }
    void setCentreDelay(float ms) { centreDelay = std::clamp(ms, 5.0f, 30.0f); }
    void setFeedback(float fb) { feedback = std::clamp(fb, 0.0f, 0.3f); }
    void setMix(float m) { mix = std::clamp(m, 0.0f, 1.0f); }

    void setParameter(float value) override {
        setDepth(value * 0.5f);
    }

private:
    double sampleRate = 44100.0;
    float maxDelayMs = 50.0f;
    std::vector<float> delayBuffer;
    int writeIndex = 0;
    
    float rate = 0.8f;
    float depth = 5.0f;
    float centreDelay = 12.0f;
    float feedback = 0.0f;
    float mix = 0.25f;
    float lfoPhase = 0.0f;
};
