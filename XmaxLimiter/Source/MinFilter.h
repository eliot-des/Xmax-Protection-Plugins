/*
  ==============================================================================

    MinFilter.h
    Created: 1 Nov 2024 7:48:35pm
    Author:  eliot
  


    Be careful with this minimum filter /!\
    ---------------------------------------
    This one is only suitable if you already know what should be the maximum 
    value in the input buffer you will process in the minimum moving filter.

    Since the gain computer output of a compressor, or limiter, is always 
    between 0.0f and 1.0f, we can use this maximum value to avoid recalculating 
    the minimum when the oldest element is the current minimum and when the 
    current minimum is the maximum value of the input buffer. Let me explain:

    When the signal doesn't exceed the threshold, the gain computer output 
    is always 1.0f, and so we don't need to recalculate the minimum value 
    when the oldest element is the current minimum thus 1.0f, which can append 
    a lot when the input signal is below the threshold.
  ==============================================================================
*/

#pragma once
#include <vector>
#include <limits>


template<typename Sample = float>
class MinFilter {
public:
    MinFilter(int maxSize) : buffer(maxSize), bufferLength(maxSize), head(0), tail(0), currentMin(std::numeric_limits<Sample>::max()) {}

    // Sets a new size for the window (resizes looking window for the minimum if needed)
    void set(int newSize) {
        // If the window size is decreasing, we need to adjust the tail and must recalculate the minimum
        if (newSize < windowSize) {
            // Move tail forward to fit the new window size
            int elementsToSkip = windowSize - newSize;
            tail = (tail + elementsToSkip) % bufferLength;
            currentSize = std::min(currentSize, newSize);
            recalculateMin();
        }
        windowSize = newSize;
    }

    // Adds a new value and updates the minimum
    void add(Sample value) {
        if (currentSize < windowSize) {
            currentSize++;
        }
        else {
            // Remove the oldest element from consideration if at window limit
            // Second condition is to avoid recalculating the minimum, 
            // but it might not be what you want. Delete "currentMin < max"
            // depending on your needs.
            if (buffer[tail] == currentMin && currentMin < max) {
                needsRecalculation = true;
            }
            tail = (tail + 1) % bufferLength;
        }

        buffer[head] = value;
        head = (head + 1) % bufferLength;

        // Update the minimum for the window
        if (value < currentMin) {
			currentMin = value;
		}
        else if (needsRecalculation) {
            recalculateMin();
		}
    }

    // Returns the current minimum of the window
    Sample getMinimum() const {
        return currentMin;
    }

    // Resets the filter
    void reset() {
        std::fill(buffer.begin(), buffer.end(), max);
        head = tail = 0;
        currentMin = max; //because the maximum value of the gain computer is 1.0f.
        currentSize = 0;
    }

    void setMax(Sample newMax) {
		max = newMax;
        recalculateMin();
	}

private:
    // Recalculates minimum on the window
    void recalculateMin() {
        currentMin = max;
        for (int i = 0; i < currentSize; ++i) {
            int index = (tail + i) % bufferLength;
            if (buffer[index] < currentMin) {
                currentMin = buffer[index];
            }
        }
        needsRecalculation = false;
    }

    std::vector<Sample> buffer;
    int bufferLength;
    int windowSize = 0;
    int head = 0;             // Index of the newest element
    int tail = 0;             // Index of the oldest element
    Sample currentMin;		  // Current minimum value in the window
    Sample max = 1.0f;		  // Maximum value that it is possible to have in the input buffer -> 1.0f if we process gain computer output
    int currentSize = 0;      // Current number of elements in the window
    bool needsRecalculation = false;
};