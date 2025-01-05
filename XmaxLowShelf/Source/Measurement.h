/*
  ==============================================================================

    Measurement.h
    Created: 6 Nov 2024 10:15:34am
    Author:  eliot

  ==============================================================================
*/

#pragma once

#include <atomic>

struct Measurement
{
    void reset() noexcept
    {
        value.store(0.0f);
    }

    void updateIfGreater(float newValue) noexcept
    {
        auto oldValue = value.load();
        while (newValue > oldValue && !value.compare_exchange_weak(oldValue, newValue));
    }

    float readAndReset() noexcept
    {
        return value.exchange(0.0f);
    }

    std::atomic<float> value;
};
