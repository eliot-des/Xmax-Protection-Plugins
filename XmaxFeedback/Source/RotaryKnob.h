/*
  ==============================================================================

    RotaryKnob.h
    Created: 2 Jan 2025 12:32:57pm
    Author:  eliot

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

class RotaryKnob : public juce::Component
{
public:
    RotaryKnob(const juce::String& text,
        juce::AudioProcessorValueTreeState& apvts,
        const juce::ParameterID& parameterID,
        bool drawFromMiddle = false);

    ~RotaryKnob() override;

    void resized() override;

    juce::Slider slider;
    juce::Label label;

    juce::AudioProcessorValueTreeState::SliderAttachment attachment;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RotaryKnob)
};
