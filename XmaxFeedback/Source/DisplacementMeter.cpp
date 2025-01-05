/*
  ==============================================================================

    DisplacementMeter.cpp
    Created: 2 Jan 2025 12:26:54pm
    Author:  eliot

  ==============================================================================
*/

#include "DisplacementMeter.h"
#include "LookAndFeel.h"

DisplacementMeter::DisplacementMeter(Measurement& measurementL_, Measurement& measurementR_, float initialThreshold)
    : measurementL(measurementL_), measurementR(measurementR_), thresholdDisplacement(initialThreshold)
{
    setOpaque(true);
    startTimerHz(refreshRate);
    decay = 1.0f - std::exp(-1.0f / (float(refreshRate) * 0.2f));

    updateTicks(); // Precompute tick values at initialization
}

DisplacementMeter::~DisplacementMeter()
{
}

void DisplacementMeter::paint(juce::Graphics& g)
{
    const auto bounds = getLocalBounds();

    g.fillAll(Colors::DisplacementMeter::background);

    drawDisplacement(g, displacementL, 0, 7);
    drawDisplacement(g, displacementR, 9, 7);

    g.setFont(Fonts::getFont(10.0f));

    // Use precomputed tick values
    for (auto& disp : tickValues) {
        int y = positionForDisplacement(disp);

        g.setColour(Colors::DisplacementMeter::tickLine);
        g.fillRect(0, y, 16, 1);
        g.setColour(Colors::DisplacementMeter::tickLabel);
        g.setFont(Fonts::getFont(9.0f));
        g.drawSingleLineText(juce::String(disp, 2), bounds.getWidth(), y + 3,
            juce::Justification::right);
    }

    g.setColour(Colors::DisplacementMeter::thresholdLine);
    g.fillRect(0, positionForDisplacement(thresholdDisplacement), 16, 1);
}

void DisplacementMeter::resized()
{
    maxPos = 4.0f;
    minPos = float(getHeight()) - 4.0f;
}

void DisplacementMeter::timerCallback()
{
    updateDisplacement(measurementL.readAndReset(), displacementL);
    updateDisplacement(measurementR.readAndReset(), displacementR);

    repaint();
}

void DisplacementMeter::drawDisplacement(juce::Graphics& g, float displacement, int x, int width)
{
    int y = juce::jlimit(0, getHeight(), positionForDisplacement(displacement));
    int y0 = juce::jlimit(0, getHeight(), positionForDisplacement(minDisplacement));

    g.setColour(Colors::DisplacementMeter::level);

    // Ensure valid fill range
    if (y < y0) g.fillRect(x, y, width, y0 - y);
}

void DisplacementMeter::updateDisplacement(float newDisplacement, float& smoothedDisplacement) const
{
    if (newDisplacement > smoothedDisplacement) {
        smoothedDisplacement = newDisplacement; // Instantaneous attack
    }
    else {
        smoothedDisplacement += (newDisplacement - smoothedDisplacement) * decay;
    }
}

void DisplacementMeter::setThresholdDisplacement(float newThreshold)
{
    thresholdDisplacement = newThreshold;

    updateTicks(); // Update tick values based on new threshold
    repaint();     // Redraw the component with updated range
}

void DisplacementMeter::updateTicks()
{
    tickValues.clear();

    float ratio = std::pow(thresholdDisplacement / minDisplacement, 1.0f / (numTicks - 1));
    for (int i = 0; i <= numTicks; ++i) {
        tickValues.push_back(minDisplacement * std::pow(ratio, i)); // Geometric progression
    }
}

int DisplacementMeter::positionForDisplacement(float displacement) const noexcept
{
    // Calculate the effective maximum displacement (including the additional tick)
    float effectiveMaxDisplacement = tickValues.back();

    // Normalize the displacement to the extended range
    float normalized = std::log10(displacement / minDisplacement) / std::log10(effectiveMaxDisplacement / minDisplacement);

    // Reverse the mapping so larger values are at the top
    return int(std::round(juce::jmap(normalized, minPos, maxPos)));
}