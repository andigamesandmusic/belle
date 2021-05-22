#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

class MusicScore;

class MainComponent   : public juce::Component
{
public:
    MainComponent();
    ~MainComponent();

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    MusicScore* score;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
