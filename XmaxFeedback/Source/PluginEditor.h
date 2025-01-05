/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "Parameters.h"
#include "RotaryKnob.h"
#include "LookAndFeel.h"
#include "LevelMeter.h"
#include "DisplacementMeter.h"

//==============================================================================
/**
*/
class XmaxFeedbackAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    XmaxFeedbackAudioProcessorEditor (XmaxFeedbackAudioProcessor&);
    ~XmaxFeedbackAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:

    //void parameterGestureChanged(int, bool) override { }

    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    XmaxFeedbackAudioProcessor& audioProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(XmaxFeedbackAudioProcessorEditor)

    RotaryKnob inputGainKnob{ "In. Gain", audioProcessor.apvts, inputGainParamID };
    RotaryKnob speakerGainKnob{ "Speaker Gain", audioProcessor.apvts, speakerGainParamID };
    RotaryKnob attackTimeKnob{ "Attack", audioProcessor.apvts, attackTimeParamID };
    RotaryKnob releaseTimeKnob{ "Release", audioProcessor.apvts, releaseTimeParamID };
    RotaryKnob thresholdDisplacementKnob{ "Threshold", audioProcessor.apvts, thresholdDisplacementParamID };
    RotaryKnob lookAheadTimeKnob{ "Look ahead", audioProcessor.apvts, lookAheadTimeParamID };
    RotaryKnob gainKnob{ "Gain", audioProcessor.apvts, gainParamID };
    RotaryKnob mixKnob{ "Mix", audioProcessor.apvts, mixParamID };

    juce::TextButton stereoButton;
    juce::AudioProcessorValueTreeState::ButtonAttachment stereoButtonAttachment{
    audioProcessor.apvts, stereoParamID.getParamID(), stereoButton
    };

    juce::Label speakerComboBoxLabel;
    juce::ComboBox speakerModelComboBox;
    juce::AudioProcessorValueTreeState::ComboBoxAttachment speakerModelComboBoxAttachment{
    audioProcessor.apvts, speakerModelParamID.getParamID(), speakerModelComboBox
    };

    juce::HyperlinkButton websiteLinkButton{
        "Eliot Deschang, Florian Marie",
        juce::URL("https://github.com/eliot-des/Xmax-Protection")
    };

    juce::GroupComponent inputGroup, speakerGroup, sideChainGroup, outputGroup;

    MainLookAndFeel mainLF;

    LevelMeter meter;
    DisplacementMeter displacementMeter;
    
};
