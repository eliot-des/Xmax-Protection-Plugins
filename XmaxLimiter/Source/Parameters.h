/*
  ==============================================================================

    Parameters.h
    Created: 31 Oct 2024 12:21:47pm
    Author:  eliot

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>


static constexpr double pi = juce::MathConstants<double>::pi;

struct LoudspeakerModel {
    float fs, Rec, Lec, Qs, Qms, Qes, Qts, Mms, Cms, Rms, Bl, Vas, Sd;

    LoudspeakerModel(float fs, float Rec, float Lec, float Qms, float Qes, float Qts,
        float Mms, float Cms, float Bl, float Vas, float Sd)
        : fs(fs), Rec(Rec), Lec(Lec), Qms(Qms), Qes(Qes), Qts(Qts), Mms(Mms),
        Cms(Cms), Bl(Bl), Vas(Vas), Sd(Sd)
    {
        Rms = 1 / (2 * pi * fs * Cms * Qms);
        Qs = std::sqrt(Mms / Cms) / (Rms + (Bl * Bl) / Rec);
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
const juce::ParameterID holdTimeParamID{ "holdTime", 1 };
const juce::ParameterID releaseTimeParamID{ "releaseTime", 1 };
//gain computer section
const juce::ParameterID limiterModeParamID{ "limiterMode", 1 };
const juce::ParameterID thresholdTensionParamID{ "thresholdTension", 1 };
const juce::ParameterID thresholdDisplacementParamID{ "thresholdDisplacement", 1 };
const juce::ParameterID kneeParamID{ "knee", 1 };
//output section
const juce::ParameterID gainParamID{ "gain", 1 };
const juce::ParameterID mixParamID{ "mix", 1 };

namespace SpeakerModels
{
    const juce::StringArray modelNames = { "Peerless HDSP830860",
                                           "Peerless Klippel",
                                            "Dayton RS150-4",
                                            "Dayton HARB252-8",
                                            "Dayton DCS165-4",
                                            "B&C 15FW76-4",
                                            "SB 10PGC21-4" };
}
namespace LimiterModes
{
	const juce::StringArray modeNames = {"Level", "Displacement" };
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

    int limiterMode = 1;
    float knee = 0.0f;
    float thresholdTension = 1.0f;
    float thresholdDisplacement = 1.0f;

    float mix = 1.0f;
    float gain = 0.0f;

    static constexpr float minAttackTime = 0.04f;
    static constexpr float maxAttackTime = 20.0f;
    static constexpr float minHoldTime = 0.0f;
    static constexpr float maxHoldTime = 100.0f;
    static constexpr float minReleaseTime = 0.0f;
    static constexpr float maxReleaseTime = 4000.0f;

    static constexpr float minTensionThreshold = 0.01f; //not zero to avoid division by zero when computing the gain reduction
    static constexpr float maxTensionThreshold = 2.0f;

    static constexpr float minDisplacementThreshold = 0.11f; //displacement in mm.
    static constexpr float maxDisplacementThreshold = 30.0f;

    juce::AudioParameterChoice* speakerModelParam;
    juce::AudioParameterChoice* limiterModeParam;
private:

    juce::AudioParameterFloat* inputGainParam;
    juce::LinearSmoothedValue<float> inputGainSmoother;
    juce::AudioParameterBool* stereoParam;

    
    juce::AudioParameterFloat* speakerGainParam;
	juce::LinearSmoothedValue<float> speakerGainSmoother;

    juce::AudioParameterFloat* thresholdDisplacementParam;
    juce::LinearSmoothedValue<float> thresholdDisplacementSmoother;
    juce::AudioParameterFloat* thresholdTensionParam;
    juce::LinearSmoothedValue<float> thresholdTensionSmoother;
    juce::AudioParameterFloat* kneeParam;
    juce::LinearSmoothedValue<float> kneeSmoother;

    juce::AudioParameterFloat* gainParam;
    juce::LinearSmoothedValue<float> gainSmoother;
    juce::AudioParameterFloat* mixParam;
    juce::LinearSmoothedValue<float> mixSmoother;

    juce::AudioParameterFloat* attackTimeParam;
    juce::AudioParameterFloat* holdTimeParam;
    juce::AudioParameterFloat* releaseTimeParam;

    float targetAttackTime = 0.0f;
    float targetHoldTime = 0.0f;
    float targetReleaseTime = 0.0f;
    float smoothingCoeff = 0.0f;  // one-pole smoothing
};