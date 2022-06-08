/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
*/
class PennyDeepReverbAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    PennyDeepReverbAudioProcessor();
    ~PennyDeepReverbAudioProcessor() override;

    //==============================================================================
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    void SetDryWetMixRatio(float ratio) {
        drywetMixer.SetMixingRatio(ratio);
    }
private:
    //Parameters
    juce::AudioProcessorValueTreeState parameters;
    std::atomic<float>* feedbackvalue{};
    std::atomic<float>* sizevalue{};
    std::atomic<float>* drywetmixratio{};
    //Var
    int sampleRate = 0, samplesPerBlock = 0;
    //Initial
    Penny::AllPassFilter<float> initialAllPass{ 2, 44110 };
    //Main
    juce::AudioBuffer<float> mainAudioBuffer{};
    juce::AudioBuffer<float> delayedMainAudioBuffer{};
    Penny::DelayLine<float> mainDelayLine{ 2, 44110 };

    Penny::AllPassFilter<float> mainAllPassReverberator0{ 2, 44110 };
    Penny::AllPassFilter<float> mainAllPassReverberator1{ 2, 44110 };
    Penny::AllPassFilter<float> mainAllPassReverberator2{ 2, 44110 };
    Penny::AllPassFilter<float> mainAllPassReverberator3{ 2, 44110 };
    Penny::AllPassFilter<float> mainAllPassReverberator4{ 2, 44110 };
    //Other
    Penny::DryWetMixer<float> drywetMixer{ 2, 44110 };
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PennyDeepReverbAudioProcessor)
};
