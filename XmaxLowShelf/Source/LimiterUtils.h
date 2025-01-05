/*
  ==============================================================================

    LimiterUtils.h
    Created: 1 Jan 2025 7:35:44pm
    Author:  eliot

  ==============================================================================
*/

#pragma once
#include <algorithm>
#include <cmath>

inline float computeGain(float x, float threshold, float knee)
{
    if (x <= threshold * (1.0f - knee / 2.0f))
    {
        return 1.0f;
    }
    else if (x > threshold * (1.0f + knee / 2.0f))
    {
        return threshold / x;
    }
    else
    {
        return 1.0f - std::pow(x - threshold + knee*threshold/2.0f, 2) / (2 * knee * threshold * x);
    }
}