/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
PennyDeepReverbAudioProcessorEditor::PennyDeepReverbAudioProcessorEditor(PennyDeepReverbAudioProcessor& p, juce::AudioProcessorValueTreeState& vts)
    : AudioProcessorEditor(&p), audioProcessor(p), valueTreeState{ vts }
{
    setSize (300, 150);

    titleLabel.setJustificationType(juce::Justification::centred);
    titleLabel.setFont(juce::Font{ 25.0f, juce::Font::bold });

    addAndMakeVisible(titleLabel);


    feedbackSliderAttachement.reset(new SliderAttachement{ valueTreeState, "FeedbackValue", reverbfeedbackSlider });
    sizeSliderAttachement.reset(new SliderAttachement{ valueTreeState, "SizeValue", reverbsizeSlider });
    drywetSliderAttachement.reset(new SliderAttachement{ valueTreeState, "DryWetMixRatio", drywetSlider });

    addAndMakeVisible(reverbfeedbackSlider);
    addAndMakeVisible(reverbsizeSlider);
    addAndMakeVisible(drywetSlider);

    reverbfeedbackLabel.setJustificationType(juce::Justification::centred);
    reverbfeedbackLabel.setFont(juce::Font{ 13.0f, juce::Font::bold });

    reverbsizeLabel.setJustificationType(juce::Justification::centred);
    reverbsizeLabel.setFont(juce::Font{ 13.0f, juce::Font::bold });

    drywetLabel.setJustificationType(juce::Justification::centred);
    drywetLabel.setFont(juce::Font{ 13.0f, juce::Font::bold });

    addAndMakeVisible(reverbfeedbackLabel);
    addAndMakeVisible(reverbsizeLabel);
    addAndMakeVisible(drywetLabel);
}

PennyDeepReverbAudioProcessorEditor::~PennyDeepReverbAudioProcessorEditor()
{
}

//==============================================================================
void PennyDeepReverbAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void PennyDeepReverbAudioProcessorEditor::resized()
{
    juce::Rectangle<int> localBounds = getLocalBounds();
    juce::Rectangle<int> headerBounds = localBounds.removeFromTop(headerHeight);
    juce::Rectangle<int> footerBounds = localBounds.removeFromBottom(footerHeight);

    titleLabel.setBounds(headerBounds);

    reverbfeedbackSlider.setBounds(localBounds.removeFromLeft(localBounds.getHeight()));
    reverbsizeSlider.setBounds(localBounds.removeFromLeft(localBounds.getHeight()));

    reverbfeedbackLabel.setBounds(footerBounds.removeFromLeft(localBounds.getHeight()));
    reverbsizeLabel.setBounds(footerBounds.removeFromLeft(localBounds.getHeight()));


    drywetSlider.setBounds(localBounds.removeFromRight(localBounds.getHeight()));
    drywetLabel.setBounds(footerBounds.removeFromRight(localBounds.getHeight()));
}
