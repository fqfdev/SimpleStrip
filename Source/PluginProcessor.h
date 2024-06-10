/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.
    Author:  Fernando Quinones Fernandez - https://fQfdev.com

 https://forum.juce.com/t/access-an-audioprocessorgraph-ptr/36330
  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Processors.h"

//==============================================================================
class StripAudioProcessor  : public juce::AudioProcessor
{
public:
    using AudioGraphIOProcessor = juce::AudioProcessorGraph::AudioGraphIOProcessor;
    using Node = juce::AudioProcessorGraph::Node;

    StripAudioProcessor();
    ~StripAudioProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //------------------------------------------------------------------------------
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //------------------------------------------------------------------------------
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    
    void lpfSetBypassed(bool isBypassed);
    void dlySetBypassed(bool isBypassed);
    
    // Oscilloscope buffer - array containing two unique pointers to float arrays.
    std::array<std::unique_ptr<float[]>, 2> visualBuffer;
    int getBufferSize(); // used to draw the oscilloscope
    
private:
    void initialiseGraph();
    void connectAudioNodes();
    void connectMidiNodes();
    
    // AudioProcessorGraph
    std::unique_ptr<juce::AudioProcessorGraph> mainProcessor;
    
    // Audiograph Nodes
    Node::Ptr filterNode;
    Node::Ptr gainNode;
    Node::Ptr audioInputNode;
    Node::Ptr audioOutputNode;
    Node::Ptr midiInputNode;
    Node::Ptr midiOutputNode;
    
    // Audio Processors
               GainProcessor* gainProcessor;
    LowpassResonantProcessor* lowPassFilter;
    
    // parameters ValueTree
    juce::AudioProcessorValueTreeState parameters;
    
    int bufferSize {0};
    
    //------------------------------------------------------------------------------
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StripAudioProcessor)
};
