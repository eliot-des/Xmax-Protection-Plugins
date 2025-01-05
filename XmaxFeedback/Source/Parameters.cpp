/*
  ==============================================================================

    Parameters.cpp
    Created: 2 Jan 2025 12:33:49pm
    Author:  eliot

  ==============================================================================
*/

#include "Parameters.h"

const std::map<juce::String, LoudspeakerModel> Parameters::speakerModelData = {
    { "Peerless HDSP830860", LoudspeakerModel(  72.0f,  6.4f, 0.278e-3f, 2.08f, 0.725f,   0.54f, 8.88e-3f,  560e-6f,  5.74f, 6.36e-3f,  89.9e-4f) },
    { "Peerless Klippel",    LoudspeakerModel(  65.5f, 7.00f, 0.515e-3f, 2.82f, 0.921f,  0.694f, 10.0e-3f,  595e-6f,  5.59f, 6.36e-3f,  89.9e-4f) },
    { "Dayton RS150-4",      LoudspeakerModel(  45.1f,  3.1f,  0.34e-3f, 1.96f,  0.40f,   0.33f,  7.7e-3f, 1.62e-3f,   4.1f, 16.4e-3f,  85.0e-4f) },
    { "Dayton HARB252-8",    LoudspeakerModel(172.11f,  7.2f,  0.09e-3f, 4.23f,  2.98f,   1.74f,  3.2e-3f,  0.3e-3f,  3.04f, 0.19e-3f,  21.2e-4f) },
    { "Dayton DCS165-4",     LoudspeakerModel(  35.7f,  3.4f,  1.43e-3f, 6.62f,  0.36f,   0.34f, 39.5e-3f,  0.5e-3f,  9.15f, 12.1e-3f, 124.7e-4f) },
    { "B&C 15FW76-4",        LoudspeakerModel(  42.0f,  3.0f,  1.04e-3f, 3.20f,  0.18f,   0.17f,  113e-3f,  131e-6f, 22.44f,   0.135f,   855e-4f) },
    { "SB 10PGC21-4",       LoudspeakerModel(   89.0f,  3.4f,  0.15e-3f, 11.2f,  1.01f,   0.92f,  2.8e-3f, 1.14e-3f,   2.3f,  1.2e-3f,    27e-4f) }
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

static juce::String stringFromVoltage(float value, int)
{
    return juce::String(value, 2) + " V";
}

Parameters::Parameters(juce::AudioProcessorValueTreeState& apvts)
{
    castParameter(apvts, inputGainParamID, inputGainParam);
    castParameter(apvts, stereoParamID, stereoParam);

    castParameter(apvts, speakerModelParamID, speakerModelParam);
    castParameter(apvts, speakerGainParamID, speakerGainParam);

    castParameter(apvts, attackTimeParamID, attackTimeParam);
    castParameter(apvts, releaseTimeParamID, releaseTimeParam);

    castParameter(apvts, thresholdDisplacementParamID, thresholdDisplacementParam);
    castParameter(apvts, lookAheadTimeParamID, lookAheadTimeParam);

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
        releaseTimeParamID,
        "Release Time",
        juce::NormalisableRange<float> { minReleaseTime, maxReleaseTime, 0.001f, 0.25f },
        1000.0f,
        juce::AudioParameterFloatAttributes().withStringFromValueFunction(stringFromMilliseconds)
    ));

    //==============================================================================

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        thresholdDisplacementParamID,
        "Displacement Threshold",
        juce::NormalisableRange<float> { minDisplacementThreshold, maxDisplacementThreshold, 0.01f, 0.25f},
        1.0f,
        juce::AudioParameterFloatAttributes().withStringFromValueFunction(stringFromMillimeters)
    ));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        lookAheadTimeParamID,
        "Look Ahead",
        juce::NormalisableRange<float> {minLookAheadTime, maxLookAheadTime, 0.001f, 0.25f },
        0.0f,
        juce::AudioParameterFloatAttributes().withStringFromValueFunction(stringFromMilliseconds)
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

    attackTime = 0.0f;
    releaseTime = 0.0f;
    lookAheadTime = 0.0f;

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
    gainSmoother.setTargetValue(juce::Decibels::decibelsToGain(gainParam->get()));
    mixSmoother.setTargetValue(mixParam->get() * 0.01f);

    targetAttackTime = attackTimeParam->get();
    if (attackTime == 0.0f) {
        attackTime = targetAttackTime;
    }
    targetReleaseTime = releaseTimeParam->get();
    if (releaseTime == 0.0f) {
        releaseTime = targetReleaseTime;
    }
    targetLookAheadTime = lookAheadTimeParam->get();
    if (lookAheadTime == 0.0f) {
        lookAheadTime = targetLookAheadTime;
	}

    speakerModel = speakerModelParam->getIndex();
}

void Parameters::smoothen() noexcept
{

    inputGain = inputGainSmoother.getNextValue();
    speakerGain = speakerGainSmoother.getNextValue();

    thresholdDisplacement = thresholdDisplacementSmoother.getNextValue();

    gain = gainSmoother.getNextValue();
    mix = mixSmoother.getNextValue();

    attackTime += (targetAttackTime - attackTime) * smoothingCoeff;
    lookAheadTime += (targetLookAheadTime - lookAheadTime) * smoothingCoeff;
    releaseTime += (targetReleaseTime - releaseTime) * smoothingCoeff;
}
