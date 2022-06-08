/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
PennyDeepReverbAudioProcessor::PennyDeepReverbAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
    parameters{ *this, nullptr, juce::Identifier{ "VTSPennyDeepReverb" }, createParameterLayout() }
#endif
{
    feedbackvalue = parameters.getRawParameterValue("FeedbackValue");
    sizevalue = parameters.getRawParameterValue("SizeValue");
    drywetmixratio = parameters.getRawParameterValue("DryWetMixRatio");
}

PennyDeepReverbAudioProcessor::~PennyDeepReverbAudioProcessor()
{
}

juce::AudioProcessorValueTreeState::ParameterLayout PennyDeepReverbAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout{};
    

    layout.add(std::make_unique<juce::AudioParameterFloat>("FeedbackValue", "Feedback", 0.0f, 1.0f, 0.5f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("SizeValue", "Size", juce::NormalisableRange<float>{ 0.0f, 1.0f, 0.0f, 2.0f }, 0.5f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("DryWetMixRatio", "Dry/Wet", 0.0f, 1.0f, 0.5f));

    return layout;
}

//==============================================================================
const juce::String PennyDeepReverbAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool PennyDeepReverbAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool PennyDeepReverbAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool PennyDeepReverbAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double PennyDeepReverbAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int PennyDeepReverbAudioProcessor::getNumPrograms()
{
    return 1; 
}

int PennyDeepReverbAudioProcessor::getCurrentProgram()
{
    return 0;
}

void PennyDeepReverbAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String PennyDeepReverbAudioProcessor::getProgramName (int index)
{
    return {};
}

void PennyDeepReverbAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void PennyDeepReverbAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    this->sampleRate = sampleRate;
    this->samplesPerBlock = samplesPerBlock;

    initialAllPass.Prepare(sampleRate, samplesPerBlock);
    initialAllPass.SetDelay(sampleRate * juce::jmap<float>(*sizevalue, 0.02f, 0.15f));
    initialAllPass.SetGain(juce::jmap<float>(*feedbackvalue, 0.25f, 0.6f));

    mainAudioBuffer.setSize(2, samplesPerBlock);
    mainAudioBuffer.clear();

    delayedMainAudioBuffer.setSize(2, samplesPerBlock);
    delayedMainAudioBuffer.clear();

    mainDelayLine.Prepare(sampleRate, samplesPerBlock);

    mainAllPassReverberator0.Prepare(sampleRate, samplesPerBlock);
    mainAllPassReverberator0.SetDelay(sampleRate * 0.0723f);
    mainAllPassReverberator0.SetGain(1);

    mainAllPassReverberator1.Prepare(sampleRate, samplesPerBlock);
    mainAllPassReverberator1.SetDelay(sampleRate * 0.0934f);
    mainAllPassReverberator1.SetGain(1);

    mainAllPassReverberator2.Prepare(sampleRate, samplesPerBlock);
    mainAllPassReverberator2.SetDelay(sampleRate * 0.0633f);
    mainAllPassReverberator2.SetGain(1);

    mainAllPassReverberator3.Prepare(sampleRate, samplesPerBlock);
    mainAllPassReverberator3.SetDelay(sampleRate * 0.0337f);
    mainAllPassReverberator3.SetGain(1);

    mainAllPassReverberator4.Prepare(sampleRate, samplesPerBlock);
    mainAllPassReverberator4.SetDelay(sampleRate * 0.1340f);
    mainAllPassReverberator4.SetGain(1);

    drywetMixer.Prepare(sampleRate, samplesPerBlock);
    drywetMixer.SetMixingRatio(*drywetmixratio);
    drywetMixer.SetMixingType(Penny::DryWetMixingType::Linear);
}

void PennyDeepReverbAudioProcessor::releaseResources()
{
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool PennyDeepReverbAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void PennyDeepReverbAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    initialAllPass.SetDelay(sampleRate * juce::jmap<float>(*sizevalue, 0.02f, 0.15f));
    initialAllPass.SetGain(juce::jmap<float>(*feedbackvalue, 0.25f, 0.6f));

    drywetMixer.SetMixingRatio(*drywetmixratio);

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    Penny::AudioBufferView<float> bufferView{ buffer };

    drywetMixer.PushDrySamples(bufferView);

    Penny::ProcessContext<float> ctx{ bufferView };

    //Initial
    initialAllPass.Process(ctx);
    //Main
    Penny::AudioBufferView<float> mainAudioBufferView{mainAudioBuffer};

    for (int i = 0; i < mainAudioBuffer.getNumChannels(); i++)
        mainAudioBuffer.copyFrom(i, 0, buffer, i, 0, buffer.getNumSamples());

    float mainGain = -juce::jmap<float>(*feedbackvalue, 0.0f, 0.9f);

    mainDelayLine.PopSamples(bufferView, sampleRate * 0.067f);

    mainAllPassReverberator0.Process(ctx);
    mainAllPassReverberator1.Process(ctx);

    mainAllPassReverberator2.Process(ctx);
    mainAllPassReverberator3.Process(ctx);
    mainAllPassReverberator4.Process(ctx);

    for (int i = 0; i < delayedMainAudioBuffer.getNumChannels(); i++)
        delayedMainAudioBuffer.copyFrom(i, 0, buffer, i, 0, buffer.getNumSamples());

    Penny::AudioBufferView<float> delayedMainAudioBufferView{ delayedMainAudioBuffer };
    delayedMainAudioBufferView *= mainGain;
    delayedMainAudioBufferView += mainAudioBufferView;

    mainDelayLine.PushSamples(delayedMainAudioBufferView);

    bufferView *= 1 - (mainGain * mainGain);
    mainAudioBufferView *= -mainGain;
    bufferView += mainAudioBufferView;

    //End
    drywetMixer.DryWetMixing(bufferView, 0);
}

//==============================================================================
bool PennyDeepReverbAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* PennyDeepReverbAudioProcessor::createEditor()
{
    return new PennyDeepReverbAudioProcessorEditor (*this, parameters);
}

//==============================================================================
void PennyDeepReverbAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    juce::ValueTree state = parameters.copyState();
    copyXmlToBinary(*state.createXml(), destData);
}

void PennyDeepReverbAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName(parameters.state.getType()))
            parameters.replaceState(juce::ValueTree::fromXml(*xmlState));
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PennyDeepReverbAudioProcessor();
}
