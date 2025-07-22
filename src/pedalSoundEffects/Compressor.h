#pragma once
#include "BaseEffects.h"
#include <cmath>
#include <algorithm>

class Compressor : public BaseEffect {
public:
    Compressor() {
        setSampleRate(44100.0);
    }

    void setSampleRate(double rate) override {
        sampleRate = rate;
        // Calculate attack and release coefficients
        updateCoefficients();
    }

    float processSample(float input) override {
        // Get the absolute value of the input
        float inputLevel = std::abs(input);
        
        // Apply attack/release envelope to create smooth level detection
        if (inputLevel > envelope) {
            // Attack phase - quickly respond to louder signals
            envelope = envelope + (inputLevel - envelope) * attackCoeff;
        } else {
            // Release phase - slowly let go when signal gets quieter
            envelope = envelope + (inputLevel - envelope) * releaseCoeff;
        }
        
        // Calculate gain reduction
        float gainReduction = 1.0f;
        
        if (envelope > threshold) {
            // Signal is above threshold - apply compression
            float overThreshold = envelope - threshold;
            float compressedLevel = threshold + (overThreshold / ratio);
            gainReduction = compressedLevel / envelope;
        }
        
        // Apply makeup gain and gain reduction
        float output = input * gainReduction * makeupGain;
        
        return output;
    }

    void setParameter(float value) override {
        // Map 0-10 to threshold (0.1 to 0.9)
        threshold = 0.1f + (value / 10.0f) * 0.8f;
    }

    void setRatio(float r) { 
        ratio = std::clamp(r, 1.0f, 10.0f); 
    }
    
    void setAttack(float a) { 
        attack = std::clamp(a, 0.001f, 0.1f);
        updateCoefficients();
    }
    
    void setMakeupGain(float gain) { 
        makeupGain = std::clamp(gain, 0.5f, 3.0f); 
    }

private:
    void updateCoefficients() {
        if (sampleRate > 0) {
            attackCoeff = 1.0f - std::exp(-1.0f / (attack * sampleRate));
            releaseCoeff = 1.0f - std::exp(-1.0f / (release * sampleRate));
        }
    }

    double sampleRate = 44100.0;
    
    // Compressor parameters
    float threshold = 0.5f;    // Level above which compression starts
    float ratio = 4.0f;        // Compression ratio (4:1)
    float attack = 0.003f;     // Attack time in seconds (3ms)
    float release = 0.1f;      // Release time in seconds (100ms)
    float makeupGain = 2.0f;   // Makeup gain to compensate for level reduction
    
    // Internal state
    float envelope = 0.0f;     // Envelope follower state
    float attackCoeff = 0.0f;  // Attack coefficient
    float releaseCoeff = 0.0f; // Release coefficient
};