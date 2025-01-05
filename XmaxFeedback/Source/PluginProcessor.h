/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Parameters.h"
#include "DelayLine.h"
#include "BiquadFilter.h"
#include "FilterDesign.h"
#include "Measurement.h"


//==============================================================================
/**
*/
class XmaxFeedbackAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    XmaxFeedbackAudioProcessor();
    ~XmaxFeedbackAudioProcessor() override;

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
    void setXuFiltersAndComputation(const LoudspeakerModel& model, float sampleRate);

    DelayLine delayLineL, delayLineR;
    BiquadFilterDF1<float> xuFilterL; // tension to displacement
    BiquadFilterDF1<float> xuFilterR;
    BiquadFilterDF1<float> xuFilterOutL; // tension to displacement
    BiquadFilterDF1<float> xuFilterOutR;


    BiquadFilterTDF2<float> compFilterL; //adaptive low shelf filter
    BiquadFilterTDF2<float> compFilterR;

    BiquadFilterTDF2<float> compDelayFilterL; //adaptive low shelf filter
    BiquadFilterTDF2<float> compDelayFilterR;

    std::function<float(float, LoudspeakerModel, float, float, float)> computeRmsComp;
 
    float Q0 = 0.707f;

    float uInL = 0.0f;
    float uInR = 0.0f;
    float uOutL = 0.0f;
    float uOutR = 0.0f;
    float uOutDelayedL = 0.0f;
    float uOutDelayedR = 0.0f;

    float xL = 0.0f;
    float xR = 0.0f;
    float xOutL = 0.0f;
    float xOutR = 0.0f;

    float CmsTargetL = 0.0f;
    float CmsTargetR = 0.0f;

    float Cthreshold = 0.5f; // CmsComp/Cms threshold ratio, used for resonant speaker RmsComp computation
    float gamma = 1.0f;

    float CmsMin = 0.0f;
    float CmsCompL = 0.0f;
    float CmsCompR = 0.0f;
    float lastCmsCompL = 0.0f;
    float lastCmsCompR = 0.0f;

    
    float RmsCompL = 0.0f;
    float RmsCompR = 0.0f;

    float wetL = 0.0f;
    float wetR = 0.0f;

    float threshold = 1.0f;
    float margin = 0.9f;

    juce::String currentSpeakerModel;
    juce::String lastSpeakerModel;
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (XmaxFeedbackAudioProcessor)
};
