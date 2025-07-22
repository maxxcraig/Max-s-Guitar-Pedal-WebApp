#include "CustomReverb.h"

CustomReverb::CustomReverb() {
    setSampleRate(44100.0);
}

void CustomReverb::setSampleRate(double rate) {
    sampleRate = rate;
    initializeDelayLines();
}

void CustomReverb::initializeDelayLines() {
    // Early reflection delay times (ms converted to samples)
    float earlyDelays[] = {19.1f, 22.6f, 28.9f, 35.8f, 41.2f, 47.3f};
    float earlyGains[] = {0.3f, 0.25f, 0.22f, 0.18f, 0.15f, 0.12f};
    
    for (int i = 0; i < 6; ++i) {
        int samples = static_cast<int>(earlyDelays[i] * sampleRate / 1000.0f);
        earlyReflections[i].buffer.assign(samples, 0.0f);
        earlyReflections[i].delaySamples = samples;
        earlyReflections[i].feedback = earlyGains[i] * decay * 0.15f;
        earlyReflections[i].writeIndex = 0;
    }
    
    // Late reverb delay times
    float lateDelays[] = {89.6f, 99.8f, 111.3f, 125.0f};
    for (int i = 0; i < 4; ++i) {
        int samples = static_cast<int>(lateDelays[i] * sampleRate / 1000.0f);
        lateReverb[i].buffer.assign(samples, 0.0f);
        lateReverb[i].delaySamples = samples;
        lateReverb[i].feedback = decay * 0.35f;
        lateReverb[i].writeIndex = 0;
    }
    
    lowpassState = 0.0f;
    for (int i = 0; i < 4; ++i) allpassStates[i] = 0.0f;
}

float CustomReverb::processSample(float input) {
    if (earlyReflections[0].buffer.empty()) return input;
    
    // Soft clip input to prevent distortion
    input = std::tanh(input * 0.8f);
    
    // Early reflections
    float early = 0.0f;
    for (int i = 0; i < 6; ++i) {
        early += earlyReflections[i].process(input * 0.3f) * 0.167f;
    }
    
    // Diffusion allpass filters with reduced gain
    float diffused = input * 0.4f + early * 0.2f;
    for (int i = 0; i < 4; ++i) {
        float delayed = allpassStates[i];
        allpassStates[i] = diffused + delayed * (diffusion * 0.5f);
        diffused = delayed - diffused * (diffusion * 0.5f);
    }
    
    // Late reverb with reduced input gain
    float late = 0.0f;
    for (int i = 0; i < 4; ++i) {
        late += lateReverb[i].process(diffused * 0.5f) * 0.25f;
    }
    
    // Tone control (simple lowpass)
    float alpha = 1.0f - tone * 0.6f;
    lowpassState = alpha * lowpassState + (1.0f - alpha) * late;
    late = lowpassState;
    
    // Mix wet and dry with increased wet gain for better audibility
    float wet = (early * 2.2f + late * 1.8f) * 0.8f;
    return std::clamp((1.0f - mix) * input + mix * wet, -1.0f, 1.0f);
}

void CustomReverb::setParameter(float value) {
    setMix(value * 0.1f);
}

void CustomReverb::setMix(float value) {
    mix = std::clamp(value, 0.0f, 1.0f);
}

void CustomReverb::setDecay(float value) {
    decay = std::clamp(value, 0.1f, 0.85f);
    // Update feedback values with reduced levels
    for (int i = 0; i < 6; ++i) {
        earlyReflections[i].feedback = (0.3f - i * 0.03f) * decay * 0.15f;
    }
    for (int i = 0; i < 4; ++i) {
        lateReverb[i].feedback = decay * 0.35f;
    }
}

void CustomReverb::setTone(float value) {
    tone = std::clamp(value, 0.0f, 1.0f);
}
