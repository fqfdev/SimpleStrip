/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.
    Author:  Fernando Quinones Fernandez - https://fQfdev.com

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Processors.h"

// TODO: refactor all of these!!!
// LPF ........................................................
static juce::String freqSliderValueToText(float value) {return juce::String(value, 0) + juce::String(" Hz");}
static float freqSliderTextToValue(const juce::String& text) {return text.getFloatValue();}
static juce::String qSliderValueToText(float value) {return juce::String(value, 2);}
static float qSliderTextToValue(const juce::String& text) {return text.getFloatValue();}

// Gain ........................................................
static float gainLevelSliderTextToValue(const juce::String& text) {return text.getFloatValue();}
static juce::String gainLevelSliderValueToText(float value) {return juce::String(value, 2) + juce::String(" x");}

juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout() 
{
    using Parameter = juce::AudioProcessorValueTreeState::Parameter;

    std::vector<std::unique_ptr<Parameter>> parameters;
    // LPF params ........................................................
    parameters.push_back(std::make_unique<Parameter> (
                     juce::String("lpfIsBypassed"), juce::String("is LPF bypassed"), juce::String(),
                     juce::NormalisableRange<float>(0.0f, 1.0f),
                     0.0f, nullptr, nullptr));
    parameters.push_back(std::make_unique<Parameter> (
                    juce::String("freq"),
                    juce::String("Cutoff Freq"),
                    juce::String(),
                    juce::NormalisableRange<float>(/*range start*/100.0f, /*range end*/15000.0f, /*increment*/ 1.0f),
                    808.0f, freqSliderValueToText, freqSliderTextToValue));
    parameters.push_back(std::make_unique<Parameter> (
                     juce::String("resonance"), juce::String("Resonance"), juce::String(),
                     juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
                     0.5f, qSliderValueToText, qSliderTextToValue));
    // Gain params ........................................................
    parameters.push_back(std::make_unique<Parameter> (
                     juce::String("gainLevel"), juce::String("Gain Level"), juce::String(),
                     juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
                     1.0f, gainLevelSliderValueToText, gainLevelSliderTextToValue));
    parameters.push_back(std::make_unique<Parameter> (
                     juce::String("gainIsBypassed"), juce::String("is Gain bypassed"), juce::String(),
                     juce::NormalisableRange<float>(0.0f, 1.0f),
                     0.0f, nullptr, nullptr));

    return { parameters.begin(), parameters.end() };
}

//==============================================================================
StripAudioProcessor::StripAudioProcessor() :
        AudioProcessor (BusesProperties().withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                                         .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
        mainProcessor  (new juce::AudioProcessorGraph()),
        parameters (*this, nullptr, juce::Identifier(JucePlugin_Name), createParameterLayout())
{

}

StripAudioProcessor::~StripAudioProcessor() {  }

// -----------------------------------------------------
void StripAudioProcessor::lpfSetBypassed(bool isBypassed)
{
    filterNode->setBypassed(isBypassed);
    gainNode->setBypassed(isBypassed); // include gain as part of LPF
}
//void StripAudioProcessor::dlySetBypassed(bool isBypassed)
//{
//    delayNode->setBypassed(isBypassed);
//}
//-----------------------------------------
void StripAudioProcessor::initialiseGraph()
{
    mainProcessor->clear();
    audioInputNode  = mainProcessor->addNode (std::make_unique<AudioGraphIOProcessor> (AudioGraphIOProcessor::audioInputNode));
    audioOutputNode = mainProcessor->addNode (std::make_unique<AudioGraphIOProcessor> (AudioGraphIOProcessor::audioOutputNode));
    midiInputNode   = mainProcessor->addNode (std::make_unique<AudioGraphIOProcessor> (AudioGraphIOProcessor::midiInputNode));
    midiOutputNode  = mainProcessor->addNode (std::make_unique<AudioGraphIOProcessor> (AudioGraphIOProcessor::midiOutputNode));
    filterNode      = mainProcessor->addNode (std::make_unique<LowpassResonantProcessor> (parameters));
    gainNode        = mainProcessor->addNode (std::make_unique<GainProcessor> (parameters));
    
    // Access the processors created by the graph nodes
    lowPassFilter       = dynamic_cast<LowpassResonantProcessor*>(filterNode->getProcessor());
    gainProcessor       = dynamic_cast<GainProcessor*>  (gainNode->getProcessor());
    
    jassert (!(lowPassFilter == nullptr || gainProcessor == nullptr)); // processor is not of types
 
    connectAudioNodes();
    connectMidiNodes();
}

void StripAudioProcessor::connectAudioNodes()
{
    // TODO: Remove channel count magic number
    for (int channel = 0; channel < 2; ++channel) {
        mainProcessor->addConnection ({ { audioInputNode->nodeID,  channel },
                                        { filterNode->nodeID,      channel } });
        mainProcessor->addConnection ({ { filterNode->nodeID,      channel },
                                        { gainNode->nodeID,       channel } });
        mainProcessor->addConnection ({ { gainNode->nodeID,        channel },
                                        { audioOutputNode->nodeID,  channel } });
    }
}

void StripAudioProcessor::connectMidiNodes()
{
    mainProcessor->addConnection (
                        { { midiInputNode->nodeID,  juce::AudioProcessorGraph::midiChannelIndex },
                          { midiOutputNode->nodeID, juce::AudioProcessorGraph::midiChannelIndex } });
}

//------------------------------------------------------------------------------
void StripAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    bufferSize = samplesPerBlock; // used to draw the oscilloscope
    
    mainProcessor->setPlayConfigDetails (getMainBusNumInputChannels(),
                                         getMainBusNumOutputChannels(),
                                         sampleRate, samplesPerBlock);

    mainProcessor->prepareToPlay (sampleRate, samplesPerBlock);

    initialiseGraph();
    
    // Oscilloscope: clear visualBuffer
    for (int channel = 0; channel < 2; channel++) {
        visualBuffer[channel].reset( new float[static_cast<int>(bufferSize) + 1]);
        for (int i = 0; i < static_cast<int>(bufferSize) + 1; i++) {
            visualBuffer[channel][i] = 0.0f;
        }
    }
}

void StripAudioProcessor::releaseResources()
{
    mainProcessor->releaseResources();
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool StripAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void StripAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals; 
    
    for (int i = getTotalNumInputChannels(); i < getTotalNumOutputChannels(); ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
    
    mainProcessor->processBlock (buffer, midiMessages);
    
    // Copy audio data to the buffer for visualization
    for (int channel = 0; channel < 2; channel++) {
        visualBuffer[channel].reset( new float[static_cast<int>(buffer.getNumSamples()) + 1]);
        auto data = buffer.getReadPointer(channel);
        for (int i = 0; i < static_cast<int>(buffer.getNumSamples()) + 1; i++) {
            visualBuffer[channel][i] = data[i];
        }
    }
 
}
//------------------------------------------------------------------------------
bool StripAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* StripAudioProcessor::createEditor()
{
    return new StripAudioProcessorEditor (*this, parameters);
}
//------------------------------------------------------------------------------
const juce::String StripAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool StripAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool StripAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool StripAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double StripAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int StripAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int StripAudioProcessor::getCurrentProgram()
{
    return 0;
}

void StripAudioProcessor::setCurrentProgram (int index) {}

const juce::String StripAudioProcessor::getProgramName (int index) {return {};}

void StripAudioProcessor::changeProgramName (int index, const juce::String& newName) {}

//==============================================================================
// The AudioProcessor::getStateInformation() callback asks your plug-in to store its state
// into a MemoryBlock object.
void StripAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
// From tutorial 0502:
        auto state = parameters.copyState();
        std::unique_ptr<juce::XmlElement> xml (state.createXml());
        copyXmlToBinary (*xml, destData);
    
}

// Here we include some error checking for safety. We also check that the ValueTree-generated XML
// is of the correct ValurTree type for our plug-in by inspecting the XML element's tag name.
void StripAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // From tutorial 0502:
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
        if (xmlState.get() != nullptr)
            if (xmlState->hasTagName (parameters.state.getType()))
                parameters.replaceState (juce::ValueTree::fromXml (*xmlState));
}

// used to draw the oscilloscope
int StripAudioProcessor::getBufferSize() { return bufferSize; }

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new StripAudioProcessor();
}
