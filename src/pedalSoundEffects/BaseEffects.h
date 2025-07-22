#pragma once

class BaseEffect
{
public:
    virtual ~BaseEffect() = default;

    //called to process a single audio sample
    virtual float processSample(float sample) = 0;

    //called when user changes control slider
    virtual void setParameter(float value) = 0;

    //set the sample rate if needed
    virtual void setSampleRate(double sampleRate) {}
};
