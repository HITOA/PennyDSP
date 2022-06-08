/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class PennyDeepReverbAudioProcessorEditor  : public juce::AudioProcessorEditor
{
private:
    using SliderAttachement = juce::AudioProcessorValueTreeState::SliderAttachment;
public:
    PennyDeepReverbAudioProcessorEditor(PennyDeepReverbAudioProcessor&, juce::AudioProcessorValueTreeState&);
    ~PennyDeepReverbAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    int headerHeight = 50;
    int footerHeight = 25;

    juce::Label titleLabel{ "titleLabel", "PENNY DEEP REVERB" };

    juce::Slider reverbfeedbackSlider{ juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::TextEntryBoxPosition::NoTextBox };
    juce::Slider reverbsizeSlider{ juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::TextEntryBoxPosition::NoTextBox };

    juce::Slider drywetSlider{ juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::TextEntryBoxPosition::NoTextBox };

    juce::Label reverbfeedbackLabel{ "reverbfeedbackLabel", "Feedback" };
    juce::Label reverbsizeLabel{ "reverbsizeLabel", "Size" };
    
    juce::Label drywetLabel{ "drywetLabel", "Dry/Wet" };

    juce::AudioProcessorValueTreeState& valueTreeState;

    std::unique_ptr<SliderAttachement> feedbackSliderAttachement;
    std::unique_ptr<SliderAttachement> sizeSliderAttachement;
    std::unique_ptr<SliderAttachement> drywetSliderAttachement;

    PennyDeepReverbAudioProcessor& audioProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PennyDeepReverbAudioProcessorEditor)
};
