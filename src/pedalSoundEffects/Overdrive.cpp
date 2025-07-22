#include "Overdrive.h"
#include <cmath>
#include <algorithm>

Overdrive::Overdrive() {}

void Overdrive::setGain(float newGain) {
    gain = std::clamp(newGain, 0.1f, 8.0f);
}

float Overdrive::processSample(float inputSample) {
    // Pre-emphasis filter to shape tone
    float preEmphasized = inputSample + 0.3f * (inputSample - lastSample);
    lastSample = inputSample;
    
    // Tube-style saturation with asymmetric clipping
    float driven = preEmphasized * gain;
    float processed;
    
    if (driven > 0.0f) {
        processed = std::tanh(driven * 0.7f);
    } else {
        processed = std::tanh(driven * 0.9f);
    }
    
    // Simple low-pass filter to smooth harsh frequencies
    processed = 0.7f * processed + 0.3f * lastOutput;
    lastOutput = processed;
    
    // Mix with dry signal and apply output level
    return (1.0f - mix) * inputSample + mix * processed * outputLevel;
}
 