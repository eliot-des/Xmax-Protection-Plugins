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
class XmaxLimiterAudioProcessorEditor : public juce::AudioProcessorEditor,
                                        private juce::AudioProcessorParameter::Listener
{
public:
    XmaxLimiterAudioProcessorEditor (XmaxLimiterAudioProcessor&);
    ~XmaxLimiterAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void parameterValueChanged(int, float) override;
    void parameterGestureChanged(int, bool) override { }
    void updateThresholdKnobs(bool displacement);

    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    XmaxLimiterAudioProcessor& audioProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (XmaxLimiterAudioProcessorEditor)


    RotaryKnob inputGainKnob{ "In. Gain", audioProcessor.apvts, inputGainParamID};
    RotaryKnob speakerGainKnob{ "Speaker Gain", audioProcessor.apvts, speakerGainParamID };
    RotaryKnob attackTimeKnob{"Attack", audioProcessor.apvts, attackTimeParamID };
    RotaryKnob holdTimeKnob{"Hold", audioProcessor.apvts, holdTimeParamID };
    RotaryKnob releaseTimeKnob{"Release", audioProcessor.apvts, releaseTimeParamID };
    RotaryKnob thresholdTensionKnob{ "Threshold", audioProcessor.apvts, thresholdTensionParamID };
    RotaryKnob thresholdDisplacementKnob{ "Threshold", audioProcessor.apvts, thresholdDisplacementParamID };
    RotaryKnob kneeKnob{ "Knee", audioProcessor.apvts, kneeParamID };
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

    juce::Label limiterComboBoxLabel;
    juce::ComboBox limiterModeComboBox;
    juce::AudioProcessorValueTreeState::ComboBoxAttachment limiterModeComboBoxAttachment{
    audioProcessor.apvts, limiterModeParamID.getParamID(), limiterModeComboBox
    };

    juce::HyperlinkButton websiteLinkButton{
        "Eliot Deschang, Florian Marie",
        juce::URL("https://github.com/eliot-des/Xmax-Protection-Plugins")
    };
    
    juce::GroupComponent inputGroup, speakerGroup, envelopeGroup, gainComputerGroup, outputGroup;

    MainLookAndFeel mainLF;

    LevelMeter meter;
    DisplacementMeter displacementMeter;
};
