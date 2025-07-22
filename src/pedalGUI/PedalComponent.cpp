#include "PedalComponent.h"

std::vector<PedalComponent*> PedalComponent::allPedals;

PedalComponent::PedalComponent(const juce::String& name, std::unique_ptr<BaseEffect> effect, const juce::String& imagePathIn)
    : pedalName(name), imagePath(imagePathIn), effectProcessor(std::move(effect))
{
    auto imageFile = juce::File::getSpecialLocation(juce::File::currentExecutableFile)
        .getParentDirectory()
        .getParentDirectory()
        .getChildFile("Resources")
        .getChildFile(imagePath);

    pedalImage = juce::ImageFileFormat::loadFrom(imageFile);

    if (!pedalImage.isValid())
        juce::Logger::writeToLog("Failed to load image: " + imageFile.getFullPathName());
    else
        juce::Logger::writeToLog("Image loaded: " + imageFile.getFullPathName());
}

PedalComponent::~PedalComponent()
{
    unregisterPedal(this);
}

void PedalComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::transparentBlack);

    if (pedalImage.isValid())
        g.drawImage(pedalImage, getLocalBounds().toFloat());
    else
    {
        g.setColour(juce::Colours::red);
        g.drawFittedText("Image not found", getLocalBounds(), juce::Justification::centred, 1);
    }
}

void PedalComponent::resized() {}

float PedalComponent::processSample(float sample)
{
    if (effectProcessor && isActive)
        return effectProcessor->processSample(sample);
    return sample;
}

void PedalComponent::setSampleRate(double rate)
{
    if (effectProcessor)
        effectProcessor->setSampleRate(rate);
}

void PedalComponent::setOriginalBounds(juce::Rectangle<int> bounds)
{
    originalBounds = bounds;
}

juce::Rectangle<int> PedalComponent::getOriginalBounds() const
{
    return originalBounds;
}

void PedalComponent::registerPedal(PedalComponent* pedal)
{
    allPedals.push_back(pedal);
}

void PedalComponent::unregisterPedal(PedalComponent* pedal)
{
    allPedals.erase(std::remove(allPedals.begin(), allPedals.end(), pedal), allPedals.end());
}

void PedalComponent::clearAllPedals()
{
    allPedals.clear();
}

std::vector<PedalComponent*> PedalComponent::getRegisteredPedals()
{
    return allPedals;
}

void PedalComponent::mouseDown(const juce::MouseEvent& event)
{
    dragOffset = event.getPosition();
    toFront(true);
}

void PedalComponent::mouseDrag(const juce::MouseEvent& event)
{
    auto newPos = getPosition() + (event.getPosition() - dragOffset);
    setTopLeftPosition(newPos);
    repaint();
}

void PedalComponent::mouseUp(const juce::MouseEvent&)
{
    for (auto* other : allPedals)
    {
        if (other != this && getBounds().intersects(other->getBounds()))
        {
            auto otherBounds = other->getBounds();
            other->setBounds(originalBounds);
            setBounds(otherBounds);

            auto temp = originalBounds;
            originalBounds = other->originalBounds;
            other->originalBounds = temp;

            return;
        }
    }

    setBounds(originalBounds);
}
