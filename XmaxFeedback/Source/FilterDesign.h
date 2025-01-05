/*
  ==============================================================================

    FilterDesign.h
    Created: 2 Jan 2025 12:29:47pm
    Author:  eliot

  ==============================================================================
*/

#pragma once
#define _USE_MATH_DEFINES
#include <array>
#include <complex>
#include <cmath>
#include <tuple>
#include <algorithm>
#include "Parameters.h"

// Normalize two arrays of coefficients by dividing each element by the first element of `a`
inline void normalize(std::array<float, 3>& a, std::array<float, 3>& b) {
    float norm = a[0];
    for (auto& coef : a) coef /= norm;
    for (auto& coef : b) coef /= norm;
}


// Bilinear transform for second-order transfer functions
inline std::pair<std::array<float, 3>, std::array<float, 3>> bilinear2ndOrder(const std::array<float, 3>& b, const std::array<float, 3>& a, float Fs) {
    std::array<float, 3> bd, ad;
    float Fs2 = Fs * Fs;

    bd[0] = b[0] * 4 * Fs2 + b[1] * 2 * Fs + b[2];
    bd[1] = -2 * b[0] * 4 * Fs2 + 2 * b[2];
    bd[2] = b[0] * 4 * Fs2 - b[1] * 2 * Fs + b[2];

    ad[0] = a[0] * 4 * Fs2 + a[1] * 2 * Fs + a[2];
    ad[1] = -2 * a[0] * 4 * Fs2 + 2 * a[2];
    ad[2] = a[0] * 4 * Fs2 - a[1] * 2 * Fs + a[2];

    normalize(ad, bd);

    return { bd, ad };
}

// Find the roots of a second-order polynomial
inline std::array<std::complex<float>, 2> roots2ndOrder(const std::array<float, 3>& coeffs) {
    float a = coeffs[0], b = coeffs[1], c = coeffs[2];
    float delta = b * b - 4 * a * c;

    if (delta > 0) {
        float r1 = (-b + std::sqrt(delta)) / (2 * a);
        float r2 = (-b - std::sqrt(delta)) / (2 * a);
        return { r1, r2 };
    }
    else if (delta == 0) {
        float r = -b / (2 * a);
        return { r, r };
    }
    else {
        std::complex<float> r1 = std::complex<float>(-b, std::sqrt(-delta)) / (2 * a);
        std::complex<float> r2 = std::complex<float>(-b, -std::sqrt(-delta)) / (2 * a);
        return { r1, r2 };
    }
}

// Convert transfer function coefficients to zeros, poles, and gain
inline std::tuple<std::array<std::complex<float>, 2>, std::array<std::complex<float>, 2>, float> tf2zpk2ndOrder(std::array<float, 3> b, std::array<float, 3> a) {
    normalize(a, b);

    float k = b[0];
    for (auto& coef : b) coef /= k;

    // Find the zeros and poles
    std::array<std::complex<float>, 2> z = roots2ndOrder(b);
    std::array<std::complex<float>, 2> p = roots2ndOrder(a);

    return { z, p, k };
}

// Convert zeros, poles, and gain back to transfer function coefficients
inline std::pair<std::array<float, 3>, std::array<float, 3>> zpk2tf2ndOrder(const std::array<std::complex<float>, 2>& z, const std::array<std::complex<float>, 2>& p, float k) {
    std::array<float, 3> b, a;

    b[0] = k;
    b[1] = -k * (z[0] + z[1]).real();
    b[2] = k * (z[0] * z[1]).real();

    a[0] = 1.0;
    a[1] = -(p[0] + p[1]).real();
    a[2] = (p[0] * p[1]).real();

    normalize(a, b);

    return { b, a };
}

// Return the 2nd order coefficients of the X/U filter of a loudspeaker with a stabilization factor, to derive a stable inverse filter U/X
inline std::pair<std::array<float, 3>, std::array<float, 3>> getXUFilterCoefficients(LoudspeakerModel model, float Fs) {

    float Rec = model.Rec;
    float Bl = model.Bl;
    float Rms = model.Rms;
    float Mms = model.Mms;
    float Cms = model.Cms;

    // Analog filter coefficients
    std::array<float, 3> b_xu = { 0.0, 0.0, Bl / Rec };
    std::array<float, 3> a_xu = { Mms, Rms + Bl * Bl / Rec, 1 / Cms };

    // Convert to digital filter
    auto coeffs = bilinear2ndOrder(b_xu, a_xu, Fs);
    auto bd_xu = coeffs.first;
    auto ad_xu = coeffs.second;

    return { bd_xu , ad_xu };
}



inline float smoothing(float x, float y, float attack, float release) {
    if (x < y)
        return (1 - attack) * y + attack * x;
    else
        return (1 - release) * y + release * x;
}

//for a non-resonant loudspeaker
inline float computeRmsComp1(float CmsComp, LoudspeakerModel model, float Q0, float /* Cthreshold */, float /* gamma */) {
    /*
    float Qc = 1 / (model.Rms + (model.Bl * model.Bl) / model.Rec) * std::sqrt(model.Mms / CmsComp);

    if (Qc < Q0)
		return std::sqrt(model.Mms / CmsComp) / Q0 - (model.Bl * model.Bl / model.Rec);
	else
		return model.Rms;
    */
    return std::max(model.Rms, std::sqrt(model.Mms / CmsComp)/Q0 - (model.Bl * model.Bl / model.Rec));
}

//for a resonant loudspeaker
inline float computeRmsComp2(float CmsComp, LoudspeakerModel model, float Q0, float Cthreshold, float gamma) {

    float QsComp = std::max(Q0, gamma * (CmsComp / model.Cms - Cthreshold) + Q0);
    return (1 / QsComp) * std::sqrt(model.Mms / CmsComp) - (model.Bl * model.Bl / model.Rec);
}

inline std::pair<std::array<float, 3>, std::array<float, 3>> getCompFilterCoeffs(LoudspeakerModel model, float CmsComp, float RmsComp, float Fs) {

    float Rec = model.Rec;
    float Bl  = model.Bl;
    float Rms = model.Rms;
    float Mms = model.Mms;
    float Cms = model.Cms;

    std::array<float, 3> b_comp = { Mms, Rms + Bl * Bl / Rec, 1 / Cms };
    std::array<float , 3> a_comp = { Mms, RmsComp + Bl * Bl / Rec, 1 / CmsComp };

    auto coeffs = bilinear2ndOrder(b_comp, a_comp, Fs);
    auto bd_comp = coeffs.first;
    auto ad_comp = coeffs.second;

    return { bd_comp , ad_comp };

}