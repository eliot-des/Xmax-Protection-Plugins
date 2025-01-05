/*
  ==============================================================================

    DelayLine.h
    Created: 6 Nov 2024 10:13:30am
    Author:  eliot

  ==============================================================================
*/

#pragma once

#include <memory>
#include <JuceHeader.h>

class DelayLine
{
public:
    // Sets the maximum delay length in samples
    void setMaximumDelayInSamples(int maxLengthInSamples) {
        jassert(maxLengthInSamples > 0);

        //Add 2 samples for future possible interpolation...
        //But here I don't interpolate, so I could remove it
        int paddedLength = maxLengthInSamples + 2;
        if (bufferLength < paddedLength) {
            bufferLength = paddedLength;
            buffer.reset(new float[size_t(bufferLength)]);
        }
    }

    // Resets the delay line
    void reset() noexcept {
        writeIndex = bufferLength - 1;
        for (size_t i = 0; i < size_t(bufferLength); ++i) {
            buffer[i] = 0.0f;
        }
    }

    // Writes an input sample to the delay line
    void write(float input) noexcept {
        jassert(bufferLength > 0);

        writeIndex += 1;
        if (writeIndex >= bufferLength) {
            writeIndex = 0;
        }

        buffer[size_t(writeIndex)] = input;
    }

    // Reads a sample from the delay line with a specified delay
    float read(int delayInSamples) const noexcept {
        jassert(delayInSamples >= 0);
        jassert(delayInSamples <= bufferLength - 1);

        int readIndex = int(std::round(writeIndex - delayInSamples));
        if (readIndex < 0) {
            readIndex += bufferLength;
        }

        return buffer[size_t(readIndex)];
    }

    // Returns the length of the buffer
    int getBufferLength() const noexcept {
        return bufferLength;
    }

private:
    std::unique_ptr<float[]> buffer;
    int bufferLength = 0;
    int writeIndex = 0;   // Index of the most recent value written
};
