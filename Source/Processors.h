/* ==============================================================================
    Processors.h
    Created: 2 Dec 2023 6:43:58pm
    Author:  Fernando Quinones Fernandez - https://fQfdev.com
  ============================================================================== */

#pragma once

//==============================================================================
class ProcessorBase : public juce::AudioProcessor
{
public:
    ProcessorBase()
        : AudioProcessor (BusesProperties().withInput ("Input", juce::AudioChannelSet::stereo())
                                           .withOutput ("Output", juce::AudioChannelSet::stereo()))
    { }

    //------------------------------------------------------------------------------
    void prepareToPlay   (double, int) override {}
    void releaseResources()            override {}
    void processBlock (juce::AudioSampleBuffer&, juce::MidiBuffer&) override {}

    //------------------------------------------------------------------------------
    juce::AudioProcessorEditor* createEditor() override          { return nullptr; }
    bool hasEditor() const override                              { return false; }

    //------------------------------------------------------------------------------
    const juce::String getName() const override                  { return {}; }
    bool acceptsMidi() const override                            { return false; }
    bool producesMidi() const override                           { return false; }
    double getTailLengthSeconds() const override                 { return 0; }

    //------------------------------------------------------------------------------
    int getNumPrograms() override                                { return 0; }
    int getCurrentProgram() override                             { return 0; }
    void setCurrentProgram (int) override                        {}
    const juce::String getProgramName (int) override             { return {}; }
    void changeProgramName (int, const juce::String&) override   {}

    //------------------------------------------------------------------------------
    void getStateInformation (juce::MemoryBlock&) override       {}
    void setStateInformation (const void*, int) override         {}

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ProcessorBase)
};

//==============================================================================
// TODO: implement bypass toggle - maybe in ProcessorBase
// simple first-order low-pass filter
class LowpassResonantProcessor : public ProcessorBase
{
public:
    LowpassResonantProcessor(juce::AudioProcessorValueTreeState& vts)
    {
        cutoffFreqParam  = vts.getRawParameterValue ("freq");
        resonanceParam   = vts.getRawParameterValue ("resonance");
    }

    ~LowpassResonantProcessor() override {}

    //------------------------------------------------------------------------------
    void prepareToPlay(double sampleRate, int samplesPerBlock) override
    {
        currentSampleRate = (float)sampleRate;
        timeIncrement = 2.0f / currentSampleRate; // time duration between two consecutive samples.
        
        cutoffFreqSmoothed.reset(sampleRate, samplesPerBlock/sampleRate);
        cutoffFreqSmoothed.setCurrentAndTargetValue(*cutoffFreqParam);
        resonanceSmoothed.reset(sampleRate, samplesPerBlock/sampleRate);
        resonanceSmoothed.setCurrentAndTargetValue(*resonanceParam);
    }

    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) override
    {
        cutoffFreqSmoothed.setTargetValue(*cutoffFreqParam);
        resonanceSmoothed.setTargetValue(*resonanceParam);
            
        auto* channelDataLeft  = buffer.getWritePointer (0);
        auto* channelDataRight = buffer.getWritePointer (1);
        
        for (int sample = 0; sample < buffer.getNumSamples(); sample++)
        {
            cutoffFreq = cutoffFreqSmoothed.getNextValue() * timeIncrement;
            resonance = resonanceSmoothed.getNextValue();
            feedback = resonance + (resonance / (1 - cutoffFreq));
                
            // left channel
            ln3 = ln3 + cutoffFreq * (channelDataLeft[sample] - ln3 + (feedback * (ln3-ln4)));
            ln4 = ln4 + cutoffFreq * (ln3-ln4);
            channelDataLeft[sample] = ln4;
                
            // right channel
            rn3= rn3 + cutoffFreq * (channelDataRight[sample] - rn3 + feedback * (rn3 - rn4));
            rn4 = rn4 + cutoffFreq * (rn3 - rn4);
            channelDataRight[sample] = rn4;
        }
    }

    void releaseResources() override {}

    const juce::String getName() const override { return "LowpassResonantProcessor"; }

private:
    std::atomic<float> *cutoffFreqParam = nullptr;
    juce::SmoothedValue<float> cutoffFreqSmoothed;
    std::atomic<float> *resonanceParam = nullptr;
    juce::SmoothedValue<float> resonanceSmoothed;

    float currentSampleRate{ 0.0 };
    float cutoffFreq {0.0}; // cut_lp
    float resonance {0.0}; // res_lp
    float timeIncrement {1.0};
    float feedback {0.0};
    float ln3, ln4 {0.0};
    float rn3, rn4 {0.0};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LowpassResonantProcessor)
};

//==============================================================================
class GainProcessor : public ProcessorBase
{
public:
    GainProcessor(juce::AudioProcessorValueTreeState& vts)
    {
        gainLevelParameter  = vts.getRawParameterValue ("gainLevel");
    }

    ~GainProcessor() override {}
    
    void prepareToPlay(double sampleRate, int samplesPerBlock) override {}

    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) override
    {
        for (int channel = 0; channel < buffer.getNumChannels(); ++channel) {
            float* channelData = buffer.getWritePointer(channel);
            
            for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
                channelData[sample] *= *gainLevelParameter;
            }
        }
    }
    
    void releaseResources() override {}

    const juce::String getName() const override { return "GainProcessor"; }

private:
    std::atomic<float> *gainLevelParameter = nullptr;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GainProcessor)
};
