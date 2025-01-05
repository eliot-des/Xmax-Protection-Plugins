/*
  ==============================================================================

    BiquadFilter.h
    Created: 6 Nov 2024 10:14:21am
    Author:  eliot

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include <array>

template<typename Sample = float>
class BiquadFilterDF1 {
public:
    void setCoefficients(const std::array<Sample, 3>& b, const std::array<Sample, 3>& a) {
        b0 = b[0];
        b1 = b[1];
        b2 = b[2];
        a0 = a[0];
        a1 = a[1];
        a2 = a[2];
    }

    Sample processSample(Sample x) {
        Sample y = b0 * x + b1 * d0 + b2 * d1 - a1 * d2 - a2 * d3;

        d1 = d0;
        d0 = x;
        d3 = d2;
        d2 = y;

        return y;
    }

    void reset() {
        d0 = d1 = d2 = d3 = 0;
    }

private:
    Sample a0 = 1, a1 = 0, a2 = 0; // Default coefficients for identity filter
    Sample b0 = 1, b1 = 0, b2 = 0;
    Sample d0 = 0, d1 = 0, d2 = 0, d3 = 0; // Delay line
};


template<typename Sample = float>
class BiquadFilterTDF2 {

public:
    void setCoefficients(const std::array<Sample, 3>& b, const std::array<Sample, 3>& a) {
        b0 = b[0];
        b1 = b[1];
        b2 = b[2];
        a0 = a[0];
        a1 = a[1];
        a2 = a[2];
    }

    Sample processSample(Sample x) {
        Sample y = b0 * x + d0;

        d0 = b1 * x - a1 * y + d1;
        d1 = b2 * x - a2 * y;

        return y;
    }

    void reset() {
        d0 = d1 = 0;
    }

private:
    Sample a0 = 1, a1 = 0, a2 = 0; // Default coefficients for identity filter
    Sample b0 = 1, b1 = 0, b2 = 0;
    Sample d0 = 0, d1 = 0; // Delay line
};
