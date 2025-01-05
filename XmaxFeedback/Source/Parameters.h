/*
  ==============================================================================

    Parameters.h
    Created: 2 Jan 2025 12:33:49pm
    Author:  eliot

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <cmath>


static constexpr double pi = juce::MathConstants<double>::pi;

struct LoudspeakerModel {
    float fs, Rec, Lec, Qs, Qms, Qes, Qts, Mms, Cms, Rms, Bl, Vas, Sd;

    LoudspeakerModel(float fs, float Rec, float Lec, float Qms, float Qes, float Qts,
        float Mms, float Cms, float Bl, float Vas, float Sd)
        : fs(fs),  Rec(Rec), Lec(Lec), Qms(Qms), Qes(Qes), Qts(Qts), Mms(Mms), 
        Cms(Cms), Bl(Bl), Vas(Vas), Sd(Sd)
    {
        Rms = 1 / (2 * pi * fs * Cms * Qms);
        Qs = std::sqrt(Mms / Cms) / (Rms + (Bl*Bl)/Rec);
    }
};

//input section
const juce::ParameterID inputGainParamID{ "inputGain", 1 };
const juce::ParameterID stereoParamID{ "stereo", 1 };
//speaker section
const juce::ParameterID speakerModelParamID{ "speakeModel", 1 };
const juce::ParameterID speakerGainParamID{ "speakerGain", 1 };
//enveloppe section
const juce::ParameterID attackTimeParamID{ "attackTime", 1 };
const juce::ParameterID releaseTimeParamID{ "releaseTime", 1 };
//gain computer section
const juce::ParameterID thresholdDisplacementParamID{ "thresholdDisplacement", 1 };
const juce::ParameterID lookAheadTimeParamID{ "lookAheadTime", 1 };
//output section
const juce::ParameterID gainParamID{ "gain", 1 };
const juce::ParameterID mixParamID{ "mix", 1 };

namespace SpeakerModels
{
    const juce::StringArray modelNames = {  "Peerless HDSP830860",
                                            "Peerless Klippel",
                                            "Dayton RS150-4", 
                                            "Dayton HARB252-8", 
                                            "Dayton DCS165-4", 
                                            "B&C 15FW76-4",
                                            "SB 10PGC21-4"};
}

class Parameters
{
public:
    Parameters(juce::AudioProcessorValueTreeState& apvts);

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    void prepareToPlay(double sampleRate) noexcept;
    void reset() noexcept;
    void update() noexcept;
    void smoothen() noexcept;

    static const std::map<juce::String, LoudspeakerModel> speakerModelData;

    float inputGain = 0.0f;
    bool stereo = true;

    int speakerModel = 0;
    float speakerGain = 0.0f;

    float attackTime = 0.02f;
    float holdTime = 0.0f;
    float releaseTime = 0.0f;

 
    float lookAheadTime = 0.0f;
    float thresholdDisplacement = 1.0f;

    float mix = 1.0f;
    float gain = 0.0f;

    static constexpr float minAttackTime = 0.04f;
    static constexpr float maxAttackTime = 20.0f;
    static constexpr float minReleaseTime = 1.0f;
    static constexpr float maxReleaseTime = 4000.0f;

    static constexpr float minLookAheadTime = 0.0f;
    static constexpr float maxLookAheadTime = 100.0f;

    static constexpr float minDisplacementThreshold = 0.11f; //displacement in mm.
    static constexpr float maxDisplacementThreshold = 30.0f;

    juce::AudioParameterChoice* speakerModelParam;
private:

    juce::AudioParameterFloat* inputGainParam;
    juce::LinearSmoothedValue<float> inputGainSmoother;
    juce::AudioParameterBool* stereoParam;

    juce::AudioParameterFloat* speakerGainParam;
    juce::LinearSmoothedValue<float> speakerGainSmoother;

    juce::AudioParameterFloat* thresholdDisplacementParam;
    juce::LinearSmoothedValue<float> thresholdDisplacementSmoother;
    juce::AudioParameterFloat* lookAheadTimeParam;

    juce::AudioParameterFloat* gainParam;
    juce::LinearSmoothedValue<float> gainSmoother;
    juce::AudioParameterFloat* mixParam;
    juce::LinearSmoothedValue<float> mixSmoother;

    juce::AudioParameterFloat* attackTimeParam;
    juce::AudioParameterFloat* releaseTimeParam;

    float targetAttackTime = 0.0f;
    float targetReleaseTime = 0.0f;
    float targetLookAheadTime = 0.0f;
    float smoothingCoeff = 0.0f;  // one-pole smoothing
};