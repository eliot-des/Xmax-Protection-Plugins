/*
  ==============================================================================

    DisplacementMeter.h
    Created: 4 Nov 2024 10:49:30pm
    Author:  eliot

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Measurement.h"

class DisplacementMeter : public juce::Component, private juce::Timer
{
public:
    DisplacementMeter(Measurement& measurementL, Measurement& measurementR, float initialThreshold);
    ~DisplacementMeter() override;

    void paint(juce::Graphics&) override;
    void resized() override;

    // Method to update the threshold displacement
    void setThresholdDisplacement(float newThreshold);

private:
    void timerCallback() override;

    // Precompute tick values
    void updateTicks();

    int positionForDisplacement(float displacement) const noexcept;

    void drawDisplacement(juce::Graphics& g, float displacement, int x, int width);
    void updateDisplacement(float newDisplacement, float& smoothedDisplacement) const;

    Measurement& measurementL;
    Measurement& measurementR;


              
    static constexpr float minDisplacement = 0.10f;              
    float thresholdDisplacement;

    float maxPos = 0.0f;
    float minPos = 0.0f;

    float displacementL = minDisplacement;
    float displacementR = minDisplacement;

    static constexpr int refreshRate = 60;
    static constexpr int numTicks = 8;

    float decay = 0.0f;

    std::vector<float> tickValues; // Store precomputed tick values

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DisplacementMeter)
};
