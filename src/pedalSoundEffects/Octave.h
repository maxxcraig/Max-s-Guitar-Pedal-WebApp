#pragma once
#include "BaseEffects.h"
#include <cmath>
#include <algorithm>
#include <vector>

class Octave : public BaseEffect {
public:
    Octave() {
        setSampleRate(44100.0);
    }

    void setSampleRate(double rate) override {
        sampleRate = rate;
        // Initialize delay buffer for octave down algorithm
        delayBuffer.resize(static_cast<size_t>(sampleRate * 0.1)); // 100ms buffer
        std::fill(delayBuffer.begin(), delayBuffer.end(), 0.0f);
        delayIndex = 0;
        
        // Reset octave generator state
        phase = 0.0f;
        lastInput = 0.0f;
        octaveSignal = 0.0f;
    }

    float processSample(float input) override {
        // Simple octave down using frequency halving approach
        
        // Store input in delay buffer for pitch tracking
        delayBuffer[delayIndex] = input;
        delayIndex = (delayIndex + 1) % delayBuffer.size();
        
        // Simple zero-crossing detection for pitch tracking
        if ((lastInput <= 0.0f && input > 0.0f) || (lastInput >= 0.0f && input < 0.0f)) {
            // Zero crossing detected - toggle octave phase
            phase = (phase > 0.0f) ? -1.0f : 1.0f;
        }
        lastInput = input;
        
        // Generate octave down signal (simple frequency division)
        octaveSignal = input * phase * octaveLevel;
        
        // Apply simple low-pass filter to octave signal to smooth it
        float filteredOctave = octaveSignal * (1.0f - toneFilter) + lastFilteredOctave * toneFilter;
        lastFilteredOctave = filteredOctave;
        
        // Mix dry and octave signals
        float drySignal = input * (1.0f - blend);
        float wetSignal = filteredOctave * blend;
        
        return drySignal + wetSignal;
    }

    void setParameter(float value) override {
        // Map 0-10 to blend range (0-100% octave mix)
        blend = (value / 10.0f);
    }

    void setBlend(float value) { blend = std::clamp(value, 0.0f, 1.0f); }
    void setTone(float value) { toneFilter = std::clamp(value, 0.0f, 0.9f); }
    void setOctaveLevel(float value) { octaveLevel = std::clamp(value, 0.0f, 2.0f); }

private:
    double sampleRate = 44100.0;
    
    // Octave generation
    std::vector<float> delayBuffer;
    size_t delayIndex = 0;
    float phase = 0.0f;
    float lastInput = 0.0f;
    float octaveSignal = 0.0f;
    
    // Filtering
    float lastFilteredOctave = 0.0f;
    
    // Parameters
    float blend = 0.5f;         // Dry/wet mix (0=all dry, 1=all octave)
    float toneFilter = 0.3f;    // Low-pass filter amount for octave signal
    float octaveLevel = 1.0f;   // Octave signal level
};