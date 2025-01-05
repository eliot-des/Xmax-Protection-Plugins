/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Parameters.h"
#include "DelayLine.h"
#include "BoxFilter.h"
#include "MinFilter.h"
#include "BiquadFilter.h"
#include "FilterDesign.h"
#include "Measurement.h"
#include "LimiterUtils.h"

//==============================================================================
/**
*/
class XmaxLimiterAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    XmaxLimiterAudioProcessor();
    ~XmaxLimiterAudioProcessor() override;

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

    juce::AudioProcessorValueTreeState apvts{
    *this, nullptr, "Parameters", Parameters::createParameterLayout()
    };


    Parameters params;

    Measurement levelL, levelR;
    Measurement displacementLevelL, displacementLevelR;

private:
    void setFiltersCoeffs(const LoudspeakerModel& model, float sampleRate);

    DelayLine delayLineL, delayLineR;
    BoxFilter<float> rectFilterL{0};
    BoxFilter<float> rectFilterR{0};
    MinFilter<float> minFilterL{0};
    MinFilter<float> minFilterR{0};
    BiquadFilterDF1<float> xuFilterL; // tension to displacement
    BiquadFilterDF1<float> xuFilterR;
    BiquadFilterDF1<float> uxFilterL; // displacement to tensions
    BiquadFilterDF1<float> uxFilterR;

    float varL=0.0f;
    float varR=0.0f;

    float gcL = 0.0f;
    float gcR = 0.0f;

    float cL=0.0f;
    float cR=0.0f;
    float gL=0.0f;
    float gR=0.0f;
    float wetL =0.0f;
    float wetR =0.0f;

    float threshold = 1.0f;
    float knee = 0.0f;
    float gain = 1.0f;

    juce::String currentSpeakerModel;
    juce::String lastSpeakerModel;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(XmaxLimiterAudioProcessor)
};
