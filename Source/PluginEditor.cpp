/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.
    Author:  Fernando Quinones Fernandez - https://fQfdev.com

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
StripAudioProcessorEditor::StripAudioProcessorEditor (StripAudioProcessor& p,
                                                      juce::AudioProcessorValueTreeState& vts )
    : AudioProcessorEditor (&p), audioProcessor (p), valueTreeState (vts)
{
    startTimer(30); // oscilloscope's refresh time
    
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;
    
    juce::LookAndFeel::setDefaultLookAndFeel(&pluginLookAndFeel);
    
    title.setText("SimpleStrip    -    fQfdev.com", juce::NotificationType::dontSendNotification);
//    title.setFont(title.getFont().withPointHeight(title.getFont().getHeightInPoints() + 4));
    title.setFont(title.getFont().withPointHeight(title.getFont().getHeightInPoints() + 1));
    title.setColour(juce::Label::textColourId, juce::Colours::white.darker(0.3));
    title.setJustificationType(juce::Justification::horizontallyCentred);
    addAndMakeVisible(&title);
    // LPF ........................................................
    freqLabel.setText("Cutoff", juce::NotificationType::dontSendNotification);
    freqLabel.setJustificationType(juce::Justification::horizontallyCentred);
    addAndMakeVisible(&freqLabel);

    /* We don't need to set up the slider's value range. This is done automatically by the SliderAttachment*/
    freqKnob.setLookAndFeel(&pluginLookAndFeel);
    freqKnob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    freqKnob.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    addAndMakeVisible(&freqKnob);
    freqAttachment.reset(new SliderAttachment(valueTreeState, "freq", freqKnob));
    
    resonanceLabel.setText("Q", juce::NotificationType::dontSendNotification);
    resonanceLabel.setJustificationType(juce::Justification::horizontallyCentred);
    addAndMakeVisible(&resonanceLabel);

    resonanceKnob.setLookAndFeel(&pluginLookAndFeel);
    resonanceKnob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    resonanceKnob.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    addAndMakeVisible(&resonanceKnob);
    resonanceAttachment.reset(new SliderAttachment(valueTreeState, "resonance", resonanceKnob));
    
    addAndMakeVisible (lpfBypassedToggle);
    lpfIsBypassed.reset(new ButtonAttachment(valueTreeState, "lpfIsBypassed", lpfBypassedToggle));
    lpfBypassedToggle.onClick = [this] { lpfIsBypassedClicked(); };
    // Gain ........................................................
    gainLevelLabel.setText("Gain", juce::NotificationType::dontSendNotification);
    gainLevelLabel.setJustificationType(juce::Justification::horizontallyCentred);
    addAndMakeVisible(&gainLevelLabel);
    
    gainLevelKnob.setLookAndFeel(&pluginLookAndFeel);
    gainLevelKnob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    gainLevelKnob.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    addAndMakeVisible(&gainLevelKnob);
    gainLevelAttachment.reset(new SliderAttachment(valueTreeState, "gainLevel", gainLevelKnob));

    setSize (400, 200);
}

StripAudioProcessorEditor::~StripAudioProcessorEditor() 
{
    setLookAndFeel(nullptr);
}

void StripAudioProcessorEditor::lpfIsBypassedClicked() {
    auto isBypassed = bool(*valueTreeState.getRawParameterValue ("lpfIsBypassed"));
    audioProcessor.lpfSetBypassed(isBypassed);
    StripAudioProcessorEditor::freqKnob.setEnabled(!isBypassed);
    StripAudioProcessorEditor::resonanceKnob.setEnabled(!isBypassed);
    StripAudioProcessorEditor::gainLevelKnob.setEnabled(!isBypassed); // include gain knob
}


//==============================================================================
void StripAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);
    
    auto area = getLocalBounds().reduced(12, 12);
    auto titleArea = area.removeFromTop(45);
    titleArea.removeFromBottom(12);

    const float width = area.getWidth();
    const float height = area.getHeight();
    const float widthThird = width / 3.0f;
//    const float heightThird = height / 3.0f;
    const float heightThird = height;
    const float tileBorderWidth = 2.0f;
    
    auto lpfArea        = area.removeFromTop(heightThird);
    auto lpfAreaLeft   = lpfArea.removeFromLeft(widthThird).reduced(tileBorderWidth,tileBorderWidth);
    auto lpfAreaCenter = lpfArea.removeFromLeft(widthThird).reduced(tileBorderWidth,tileBorderWidth);
    auto gainArea      = lpfArea.removeFromLeft(widthThird).reduced(tileBorderWidth,tileBorderWidth);
    
    auto tileColor = juce::Colour(207, 177, 86);
    g.setColour (tileColor);
    g.fillRect (lpfAreaLeft);
    g.fillRect (lpfAreaCenter);
    g.fillRect (gainArea);
    
}


//--------------------------------------------------------------------------------------
void StripAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced(12, 12);
    auto titleArea = area.removeFromTop(45);
    titleArea.removeFromBottom(12);
    
    const float width = area.getWidth();
    const float height = area.getHeight();
    const float widthThird = width / 3;
//    const float heightThird = height / 3;
    const float heightThird = height;
    const float tileBorderWidth = 2.0f;
    
    auto lpfArea            = area.removeFromTop(heightThird);
    auto lpfAreaLeft        = lpfArea.removeFromLeft(widthThird).reduced(tileBorderWidth,tileBorderWidth);
    auto lpfAreaCenter      = lpfArea.removeFromLeft(widthThird).reduced(tileBorderWidth,tileBorderWidth);
    auto gainArea           = lpfArea.removeFromLeft(widthThird).reduced(tileBorderWidth,tileBorderWidth);

    title.setBounds(titleArea);

    freqLabel.setBounds(lpfAreaLeft.removeFromBottom(20));
    freqKnob.setBounds(lpfAreaLeft);
    lpfBypassedToggle.setBounds(lpfAreaLeft.getX(), lpfAreaLeft.getY(), 20, 20);
    resonanceLabel.setBounds(lpfAreaCenter.removeFromBottom(20));
    resonanceKnob.setBounds(lpfAreaCenter);
    // Gain
    gainLevelLabel.setBounds(gainArea.removeFromBottom(20));
    gainLevelKnob.setBounds(gainArea);
}

//--------------------------------------------------------------------------------------
void StripAudioProcessorEditor::timerCallback()
{
    // This method is called when the timer triggers
    repaint(); // Request a repaint to update the graphics
}
