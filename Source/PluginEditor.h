/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.
    Author:  Fernando Quinones Fernandez - https://fQfdev.com

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class StripAudioProcessorEditor  : public juce::AudioProcessorEditor,
                                   private juce::Timer
{
public:
    StripAudioProcessorEditor (StripAudioProcessor&, juce::AudioProcessorValueTreeState& vts);
    ~StripAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    void timerCallback() override; // repaint oscilloscope

private:
    
    void gainIsBypassedClicked();
    void lpfIsBypassedClicked();
    void dlyIsBypassedClicked();
    
    StripAudioProcessor& audioProcessor;

    juce::LookAndFeel_V4 pluginLookAndFeel;
    juce::AudioProcessorValueTreeState& valueTreeState;
    juce::Label title;
    
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;
    // LPF ........................................................
    juce::ToggleButton lpfBypassedToggle { "LPF Bypass" };
    std::unique_ptr<ButtonAttachment> lpfIsBypassed;
    
    juce::Label  freqLabel;
    juce::Slider freqKnob;
    std::unique_ptr<SliderAttachment> freqAttachment;

    juce::Label  resonanceLabel;
    juce::Slider resonanceKnob;
    std::unique_ptr<SliderAttachment> resonanceAttachment;
    // Gain ........................................................
    juce::Label  gainLevelLabel;
    juce::Slider gainLevelKnob;
    std::unique_ptr<SliderAttachment> gainLevelAttachment;
    
    void drawWaveform(juce::Graphics& g, const float* data, juce::Colour color, int yOffset, juce::Rectangle<int> area);
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StripAudioProcessorEditor)
};
