#include "PedalBoardScreen.h"

PedalBoardScreen::PedalBoardScreen(std::function<void()> goHome)
    : onBackToHome(goHome)
{
    std::cout << "[PedalBoardScreen] Constructor started" << std::endl;
    
    // Clear any stale pedal registrations from previous instances
    PedalComponent::clearAllPedals();

    juce::RuntimePermissions::request(juce::RuntimePermissions::recordAudio,
        [](bool granted) {
            if (!granted)
                juce::Logger::outputDebugString("Mic permission not granted!");
        });

    std::cout << "Loading background image..." << std::endl;
    backgroundImage = juce::ImageCache::getFromFile(
        getAppBundleResource("Pedals.png"));

    std::cout << "Creating pedals..." << std::endl;
    overdrivePedal = std::make_unique<OverdriveComponent>();
    reverbPedal    = std::make_unique<ReverbComponent>();
    chorusPedal    = std::make_unique<ChorusComponent>();
    bluesDriverPedal = std::make_unique<BluesDriverComponent>();
    distortionPedal  = std::make_unique<DistortionComponent>();
    delayPedal       = std::make_unique<CompressorComponent>();
    tremoloPedal     = std::make_unique<TremoloComponent>();
    phaserPedal      = std::make_unique<OctaveComponent>();

    std::cout << "Adding pedals to screen..." << std::endl;
    addAndMakeVisible(*overdrivePedal);
    addAndMakeVisible(*reverbPedal);
    addAndMakeVisible(*chorusPedal);
    addAndMakeVisible(*bluesDriverPedal);
    addAndMakeVisible(*distortionPedal);
    addAndMakeVisible(*delayPedal);
    addAndMakeVisible(*tremoloPedal);
    addAndMakeVisible(*phaserPedal);

    std::cout << "Setting up buttons..." << std::endl;
    addAndMakeVisible(backButton);
    backButton.setColour(juce::TextButton::buttonColourId, juce::Colours::grey);
    backButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    backButton.setButtonText("Home");
    backButton.onClick = [this]() { onBackToHome(); };

    addAndMakeVisible(saveButton);
    saveButton.setColour(juce::TextButton::buttonColourId, juce::Colours::grey);
    saveButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    saveButton.setButtonText("Save Board");
    saveButton.onClick = [this]() {
        // Check if user is authenticated
        auto& authManager = AuthManager::getInstance();
        if (!authManager.isLoggedIn()) {
            auto* authAlert = new juce::AlertWindow("Authentication Required", 
                "Please log in to save boards to the cloud.", 
                juce::AlertWindow::WarningIcon);
            authAlert->addButton("OK", 1);
            authAlert->enterModalState(true, juce::ModalCallbackFunction::create(
                [authAlert](int) {
                    std::unique_ptr<juce::AlertWindow> cleanup(authAlert);
                }), false);
            return;
        }
        
        auto* alert = new juce::AlertWindow("Save Board", "Enter a name for your board:", juce::AlertWindow::NoIcon);
        alert->addTextEditor("boardName", "MyBoard", "Board Name:");
        alert->addButton("Save", 1);
        alert->addButton("Cancel", 0);

        alert->enterModalState(true, juce::ModalCallbackFunction::create(
            [this, &authManager, alert](int result)
            {
                std::unique_ptr<juce::AlertWindow> cleanup(alert);
                if (result == 1)
                {
                    juce::String boardName = alert->getTextEditor("boardName")->getText();
                    if (boardName.isNotEmpty()) {
                        // Collect pedal data
                        juce::Array<juce::var> pedals;
                        for (auto* pedal : PedalComponent::getRegisteredPedals()) {
                            juce::DynamicObject::Ptr p = new juce::DynamicObject();
                            p->setProperty("type", pedal->getName());
                            p->setProperty("enabled", pedal->isEnabled());
                            p->setProperty("x", pedal->getX());
                            p->setProperty("y", pedal->getY());
                            
                            // Save knob states
                            juce::var knobStates = pedal->getKnobStates();
                            if (!knobStates.isVoid())
                                p->setProperty("knobStates", knobStates);
                                
                            pedals.add(juce::var(p));
                        }

                        // Create board data structure
                        juce::DynamicObject::Ptr data = new juce::DynamicObject();
                        data->setProperty("name", boardName);
                        data->setProperty("pedals", pedals);
                        juce::var boardData(data);

                        // Save to cloud using Supabase
                        juce::String accessToken = authManager.getAccessToken();
                        juce::String userId = authManager.getCurrentUserId();
                        BoardResponse response = supabaseClient.saveBoard(accessToken, userId, boardName, boardData);
                        
                        // Show result to user
                        juce::String title = response.success ? "Save Successful" : "Save Failed";
                        juce::String message = response.success ? 
                            "Board '" + boardName + "' saved successfully!" : 
                            "Failed to save board: " + response.errorMessage;
                        
                        auto* resultAlert = new juce::AlertWindow(title, message, 
                            response.success ? juce::AlertWindow::InfoIcon : juce::AlertWindow::WarningIcon);
                        resultAlert->addButton("OK", 1);
                        resultAlert->enterModalState(true, juce::ModalCallbackFunction::create(
                            [resultAlert](int) {
                                std::unique_ptr<juce::AlertWindow> cleanup(resultAlert);
                            }), false);
                    }
                }
            }), false);
    };

    // Setup bypass button
    addAndMakeVisible(bypassButton);
    bypassButton.setColour(juce::TextButton::buttonColourId, juce::Colours::red);
    bypassButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    bypassButton.onClick = [this]() {
        masterBypass = !masterBypass;
        bypassButton.setButtonText(masterBypass ? "Effects Off" : "Bypass All");
        bypassButton.setColour(juce::TextButton::buttonColourId, 
                              masterBypass ? juce::Colours::green : juce::Colours::red);
    };

    // Setup input gain slider
    addAndMakeVisible(inputGainSlider);
    addAndMakeVisible(inputGainLabel);
    inputGainSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    inputGainSlider.setRange(0.0, 1.0, 0.01);
    inputGainSlider.setValue(0.3);
    inputGainSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 50, 20);
    inputGainSlider.onValueChange = [this]() {
        inputGain = inputGainSlider.getValue();
    };
    inputGainLabel.setColour(juce::Label::textColourId, juce::Colours::white);

    std::cout << "Constructor complete. Initializing audio." << std::endl;
    setAudioChannels(2, 2);
    setSize(1000, 600);
}


PedalBoardScreen::~PedalBoardScreen() {
    shutdownAudio();
}

void PedalBoardScreen::loadBoardData(const juce::var& boardVar)
{
    std::cout << "[loadBoardData] entered" << std::endl;
    std::cout << "[loadBoardData] entered" << std::endl;


    if (auto* obj = boardVar.getDynamicObject())
    {
        std::cout << "[loadBoardData] got dynamic object" << std::endl;

        juce::var pedalsVar = obj->getProperty("pedals");
        std::cout << "[loadBoardData] got pedals property" << std::endl;

        if (auto* pedalArray = pedalsVar.getArray())
        {
            std::cout << "[loadBoardData] got pedal array, size: " << pedalArray->size() << std::endl;

            for (auto& pedalEntry : *pedalArray)
            {
                std::cout << "[loadBoardData] parsing pedal entry" << std::endl;

                if (auto* p = pedalEntry.getDynamicObject())
                {
                    juce::String type = p->getProperty("type").toString();
                    bool enabled = p->getProperty("enabled");
                    int x = (int) p->getProperty("x");
                    int y = (int) p->getProperty("y");

                    std::cout << "[loadBoardData] trying pedal type: " << type << std::endl;

                    if (auto* pedal = getPedalByName(type))
                    {
                        std::cout << "[loadBoardData] matched pedal: " << type << std::endl;
                        juce::Rectangle<int> newBounds(x, y, pedal->getWidth(), pedal->getHeight());
                        pedal->setBounds(newBounds);
                        pedal->setOriginalBounds(newBounds); // Update originalBounds for drag-and-drop
                        pedal->setEnabled(enabled);
                        
                        // Load knob states if available
                        if (p->hasProperty("knobStates"))
                        {
                            juce::var knobStates = p->getProperty("knobStates");
                            pedal->setKnobStates(knobStates);
                            std::cout << "[loadBoardData] loaded knob states for: " << type << std::endl;
                        }
                    }
                    else
                    {
                        std::cout << "[loadBoardData] no match for: " << type << std::endl;
                    }
                }
                else
                {
                    std::cout << "[loadBoardData] failed to get dynamic object for pedalEntry" << std::endl;
                }
            }
        }
        else
        {
            std::cout << "[loadBoardData] pedals property is not an array" << std::endl;
        }
    }
    else
    {
        std::cout << "[loadBoardData] boardVar is not a dynamic object" << std::endl;
    }
    std::cout << "[loadBoardData] exiting cleanly" << std::endl;

}






PedalComponent* PedalBoardScreen::getPedalByName(const juce::String& name)
{
    if (overdrivePedal && overdrivePedal->getName() == name) return overdrivePedal.get();
    if (distortionPedal && distortionPedal->getName() == name) return distortionPedal.get();
    if (reverbPedal && reverbPedal->getName() == name) return reverbPedal.get();
    if (chorusPedal && chorusPedal->getName() == name) return chorusPedal.get();
    if (bluesDriverPedal && bluesDriverPedal->getName() == name) return bluesDriverPedal.get();
    if (delayPedal && delayPedal->getName() == name) return delayPedal.get();
    if (tremoloPedal && tremoloPedal->getName() == name) return tremoloPedal.get();
    if (phaserPedal && phaserPedal->getName() == name) return phaserPedal.get();
    return nullptr;
}


void PedalBoardScreen::prepareToPlay(int samplesPerBlockExpected, double sampleRate) {
    currentSampleRate = sampleRate;
    auto setSR = [=](auto& pedal) {
        if (pedal) pedal->setSampleRate(sampleRate);
    };
    setSR(overdrivePedal);
    setSR(reverbPedal);
    setSR(chorusPedal);
    setSR(bluesDriverPedal);
    setSR(distortionPedal);
    setSR(delayPedal);
    setSR(tremoloPedal);
    setSR(phaserPedal);
}

void PedalBoardScreen::getNextAudioBlock(const juce::AudioSourceChannelInfo& info) {
    auto totalInputChannels  = deviceManager.getCurrentAudioDevice()->getActiveInputChannels().countNumberOfSetBits();
    auto totalOutputChannels = deviceManager.getCurrentAudioDevice()->getActiveOutputChannels().countNumberOfSetBits();

    for (int channel = 0; channel < totalOutputChannels; ++channel) {
        auto* outBuffer = info.buffer->getWritePointer(channel, info.startSample);
        if (channel < totalInputChannels) {
            const float* inBuffer = info.buffer->getReadPointer(channel, info.startSample);
            for (int i = 0; i < info.numSamples; ++i) {
                // Apply input gain with better scaling
                float sample = inBuffer[i] * inputGain;
                
                // Process through pedals only if not bypassed
                if (!masterBypass) {
                    auto processModulation = [&](auto& pedal) {
                        if (pedal && pedal->isEnabled()) {
                            sample = pedal->processSample(sample);
                            sample *= 0.9f; // Less attenuation for modulation
                        }
                    };
                    auto processDistortion = [&](auto& pedal) {
                        if (pedal && pedal->isEnabled()) {
                            sample = pedal->processSample(sample);
                            sample *= 0.8f; // More attenuation for distortion
                        }
                    };
                    auto processTimeBase = [&](auto& pedal) {
                        if (pedal && pedal->isEnabled()) {
                            sample = pedal->processSample(sample);
                            sample *= 0.85f; // Medium attenuation for time-based
                        }
                    };
                    
                    // Proper signal chain: Modulation → Distortion → Time-based
                    processModulation(chorusPedal);
                    processModulation(phaserPedal);
                    processDistortion(overdrivePedal);
                    processDistortion(distortionPedal);
                    processDistortion(bluesDriverPedal);
                    processModulation(tremoloPedal);
                    processTimeBase(delayPedal);
                    processTimeBase(reverbPedal);
                }
                
                // Apply soft limiting instead of hard clipping
                sample = softLimit(sample);
                
                // Final output scaling
                outBuffer[i] = sample * 0.7f;
            }
        } else {
            juce::FloatVectorOperations::clear(outBuffer, info.numSamples);
        }
    }
}

void PedalBoardScreen::releaseResources() {}

float PedalBoardScreen::softLimit(float sample) {
    // Soft limiting using tanh for musical distortion
    if (std::abs(sample) > 0.7f) {
        return std::tanh(sample * 0.7f) / 0.7f;
    }
    return sample;
}

void PedalBoardScreen::paint(juce::Graphics& g) {
    if (backgroundImage.isValid())
        g.drawImage(backgroundImage, getLocalBounds().toFloat());
    else
        g.fillAll(juce::Colours::darkgrey);

    g.setColour(juce::Colours::white);
    g.setFont(juce::Font().withHeight(40.0f).boldened());
    g.drawText("Guitar Pedal Board", getLocalBounds().removeFromTop(80), juce::Justification::centred, true);

    g.setFont(juce::Font().withHeight(14.0f));
    g.drawText("*If you have your mic as the input and output and no guitar, you'll create a feedback loop that sounds terrible. This is NOT intended to be used without a Guitar.",
               getLocalBounds().removeFromBottom(60).reduced(10),
               juce::Justification::centred, true);
    g.drawText("*Press pedal to turn on, white = on",
               getLocalBounds().removeFromBottom(30).reduced(10),
               juce::Justification::centred, true);
}

void PedalBoardScreen::resized() {
    const int pedalWidth = 200;
    const int pedalHeight = 160;
    const int padding = 20;
    const int columns = 4;
    const int titleHeight = 100;
    auto totalWidth = columns * pedalWidth + (columns + 1) * padding;
    int startX = (getWidth() - totalWidth) / 2 + padding;

    backButton.setBounds(10, 10, 100, 35);
    saveButton.setBounds(getWidth() - 110, 10, 100, 35);
    bypassButton.setBounds(10, 50, 120, 35);
    
    // Position input gain controls in top right
    inputGainLabel.setBounds(getWidth() - 250, 50, 80, 20);
    inputGainSlider.setBounds(getWidth() - 160, 50, 150, 35);

    auto placePedal = [&](auto& pedal, int row, int col) {
        if (pedal) {
            pedal->setBounds(startX + col * (pedalWidth + padding),
                             titleHeight + padding + row * (pedalHeight + padding),
                             pedalWidth, pedalHeight);
            pedal->setOriginalBounds(pedal->getBounds());
            PedalComponent::registerPedal(pedal.get());
        }
    };

    placePedal(overdrivePedal, 0, 0);
    placePedal(reverbPedal, 0, 1);
    placePedal(distortionPedal, 0, 2);
    placePedal(bluesDriverPedal, 0, 3);
    placePedal(delayPedal, 1, 0);
    placePedal(tremoloPedal, 1, 1);
    placePedal(chorusPedal, 1, 2);
    placePedal(phaserPedal, 1, 3);
}
