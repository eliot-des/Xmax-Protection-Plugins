/*
  ==============================================================================

    Parameters.cpp
    Created: 6 Nov 2024 10:12:45am
    Author:  eliot

  ==============================================================================
*/

#include "Parameters.h"

const std::map<juce::String, LoudspeakerModel> Parameters::speakerModelData = {
    { "Peerless HDSP830860", LoudspeakerModel(72.0,  6.4, 0.278e-3, 2.08, 0.725, 0.54, 8.88e-3,  560e-6, 5.74, 6.36e-3, 89.9e-4) },
    { "Peerless Klippel",    LoudspeakerModel(65.5, 7.00, 0.515e-3, 2.82,  0.921, 0.694, 10.0e-3, 595e-6,  5.594,  6.36e-3,  89.9e-4) },
    { "Dayton RS150-4",      LoudspeakerModel(45.1,  3.1, 0.34e-3 , 1.96,  0.40, 0.33,  7.7e-3, 1.62e-3,  4.1, 16.4e-3, 85.0e-4) },
    { "Dayton HARB252-8",    LoudspeakerModel(172.11,7.2,  0.09e-3, 4.23,  2.98, 1.74,  3.2e-3,  0.3e-3, 3.04, 0.19e-3, 21.2e-4) },
    { "Dayton DCS165-4",     LoudspeakerModel(35.7,  3.4,  1.43e-3, 6.62,  0.36, 0.34, 39.5e-3,  0.5e-3, 9.15, 12.1e-3,124.7e-4) },
    { "B&C 15FW76-4",        LoudspeakerModel(  42,   3.0, 1.04e-3, 3.20,  0.18, 0.17,  113e-3,  131e-6,22.44,   0.135,  855e-4) },
    { "SB 10PGC21-4",       LoudspeakerModel(  89,   3.4,  0.15e-3, 11.2,  1.01, 0.92,  2.8e-3, 1.14e-3,  2.3,  1.2e-3,   27e-4) }
};

template<typename T>
static void castParameter(juce::AudioProcessorValueTreeState& apvts,
    const juce::ParameterID& id, T& destination)
{
    destination = dynamic_cast<T>(apvts.getParameter(id.getParamID()));
    jassert(destination);  // parameter does not exist or wrong type
}

static juce::String stringFromMilliseconds(float value, int)
{
    if (value < 10.0f) {
        return juce::String(value, 2) + " ms";
    }
    else if (value < 100.0f) {
        return juce::String(value, 1) + " ms";
    }
    else if (value < 1000.0f) {
        return juce::String(int(value)) + " ms";
    }
    else {
        return juce::String(value * 0.001f, 2) + " s";
    }
}

static juce::String stringFromMillimeters(float value, int)
{
    return juce::String(value, 2) + " mm";
}


static juce::String stringFromDecibels(float value, int)
{
    return juce::String(value, 1) + " dB";
}

static juce::String stringFromPercent(float value, int)
{
    return juce::String(int(value)) + " %";
}


Parameters::Parameters(juce::AudioProcessorValueTreeState& apvts)
{
    castParameter(apvts, inputGainParamID, inputGainParam);
    castParameter(apvts, stereoParamID, stereoParam);

    castParameter(apvts, speakerModelParamID, speakerModelParam);
    castParameter(apvts, speakerGainParamID, speakerGainParam);

    castParameter(apvts, attackTimeParamID, attackTimeParam);
    castParameter(apvts, holdTimeParamID, holdTimeParam);
    castParameter(apvts, releaseTimeParamID, releaseTimeParam);

    castParameter(apvts, filterModeParamID, filterModeParam);
    castParameter(apvts, thresholdDisplacementParamID, thresholdDisplacementParam);
    castParameter(apvts, kneeParamID, kneeParam);

    castParameter(apvts, mixParamID, mixParam);
    castParameter(apvts, gainParamID, gainParam);
}

juce::AudioProcessorValueTreeState::ParameterLayout Parameters::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;


    layout.add(std::make_unique<juce::AudioParameterFloat>(
        inputGainParamID,
        "Input Gain",
        juce::NormalisableRange<float> { -12.0f, 12.0f },
        0.0f,
        juce::AudioParameterFloatAttributes().withStringFromValueFunction(stringFromDecibels)
    ));

    layout.add(std::make_unique<juce::AudioParameterBool>(
        stereoParamID, "Stereo", true));

    //==============================================================================

    layout.add(std::make_unique<juce::AudioParameterChoice>(
        speakerModelParamID, "Speaker Model", SpeakerModels::modelNames, 0));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        speakerGainParamID,
        "Speaker Gain",
        juce::NormalisableRange<float> {0.0f, 60.0f },
        0.0f,
        juce::AudioParameterFloatAttributes().withStringFromValueFunction(stringFromDecibels)
    ));

    //==============================================================================
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        attackTimeParamID,
        "Attack Time",
        juce::NormalisableRange<float> { minAttackTime, maxAttackTime, 0.001f, 0.25f },
        3.0f,
        juce::AudioParameterFloatAttributes().withStringFromValueFunction(stringFromMilliseconds)
    ));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        holdTimeParamID,
        "Hold Time",
        juce::NormalisableRange<float> { minHoldTime, maxHoldTime, 0.001f, 0.25f },
        10.0f,
        juce::AudioParameterFloatAttributes().withStringFromValueFunction(stringFromMilliseconds)
    ));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        releaseTimeParamID,
        "Release Time",
        juce::NormalisableRange<float> { minReleaseTime, maxReleaseTime, 0.001f, 0.25f },
        85.0f,
        juce::AudioParameterFloatAttributes().withStringFromValueFunction(stringFromMilliseconds)
    ));

    //==============================================================================

    layout.add(std::make_unique<juce::AudioParameterChoice>(
        filterModeParamID, "Filter mode", FilterModes::modeNames, 0));


    layout.add(std::make_unique<juce::AudioParameterFloat>(
        thresholdDisplacementParamID,
        "Displacement Threshold",
        juce::NormalisableRange<float> { minDisplacementThreshold, maxDisplacementThreshold, 0.01f, 0.25f},
        1.0f,
        juce::AudioParameterFloatAttributes().withStringFromValueFunction(stringFromMillimeters)
    ));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        kneeParamID,
        "Knee",
        juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f),
        0.0f,
        juce::AudioParameterFloatAttributes().withStringFromValueFunction(stringFromPercent)
    ));

    //==============================================================================
    layout.add(std::make_unique<juce::AudioParameterFloat>(
        gainParamID,
        "Output Gain",
        juce::NormalisableRange<float> { -12.0f, 12.0f },
        0.0f,
        juce::AudioParameterFloatAttributes().withStringFromValueFunction(stringFromDecibels)
    ));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        mixParamID,
        "Mix",
        juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f),
        100.0f,
        juce::AudioParameterFloatAttributes().withStringFromValueFunction(stringFromPercent)
    ));

    return layout;
}

void Parameters::prepareToPlay(double sampleRate) noexcept
{
    double duration = 0.02;

    inputGainSmoother.reset(sampleRate, duration);
    speakerGainSmoother.reset(sampleRate, duration);
    thresholdDisplacementSmoother.reset(sampleRate, duration);
    kneeSmoother.reset(sampleRate, duration);
    gainSmoother.reset(sampleRate, duration);
    mixSmoother.reset(sampleRate, duration);


    smoothingCoeff = 1.0f - std::exp(-1.0f / (0.1f * float(sampleRate)));
}

void Parameters::reset() noexcept
{
    inputGain = 0.0f;
    inputGainSmoother.setCurrentAndTargetValue(juce::Decibels::decibelsToGain(inputGainParam->get()));

    speakerGain = 0.0f;
    speakerGainSmoother.setCurrentAndTargetValue(juce::Decibels::decibelsToGain(speakerGainParam->get()));

    thresholdDisplacement = 1.0f;
    thresholdDisplacementSmoother.setCurrentAndTargetValue(thresholdDisplacementParam->get());
    knee = 0.0f;
    kneeSmoother.setCurrentAndTargetValue(kneeParam->get());


    attackTime = 0.0f;
    holdTime = 0.0f;
    releaseTime = 0.0f;

    gain = 0.0f;
    gainSmoother.setCurrentAndTargetValue(juce::Decibels::decibelsToGain(gainParam->get()));
    mix = 1.0f;
    mixSmoother.setCurrentAndTargetValue(mixParam->get() * 0.01f);
}

void Parameters::update() noexcept
{

    inputGainSmoother.setTargetValue(juce::Decibels::decibelsToGain(inputGainParam->get()));
    speakerGainSmoother.setTargetValue(juce::Decibels::decibelsToGain(speakerGainParam->get()));
    thresholdDisplacementSmoother.setTargetValue(thresholdDisplacementParam->get());
    kneeSmoother.setTargetValue(kneeParam->get() * 0.01f);
    gainSmoother.setTargetValue(juce::Decibels::decibelsToGain(gainParam->get()));
    mixSmoother.setTargetValue(mixParam->get() * 0.01f);

    targetAttackTime = attackTimeParam->get();
    if (attackTime == 0.0f) {
        attackTime = targetAttackTime;
    }
    targetHoldTime = holdTimeParam->get();
    if (holdTime == 0.0f) {
        holdTime = targetHoldTime;
    }
    targetReleaseTime = releaseTimeParam->get();
    if (releaseTime == 0.0f) {
        releaseTime = targetReleaseTime;
    }

    filterMode = filterModeParam->getIndex();
    speakerModel = speakerModelParam->getIndex();
}

void Parameters::smoothen() noexcept
{

    inputGain = inputGainSmoother.getNextValue();
    speakerGain = speakerGainSmoother.getNextValue();

    thresholdDisplacement = thresholdDisplacementSmoother.getNextValue();
    knee = kneeSmoother.getNextValue();

    gain = gainSmoother.getNextValue();
    mix = mixSmoother.getNextValue();

    attackTime += (targetAttackTime - attackTime) * smoothingCoeff;
    holdTime += (targetHoldTime - holdTime) * smoothingCoeff;
    releaseTime += (targetReleaseTime - releaseTime) * smoothingCoeff;
}
