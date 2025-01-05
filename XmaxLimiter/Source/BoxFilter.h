/*
  ==============================================================================

    BoxFilter.h
    Created: 1 Nov 2024 5:33:58pm
    Author: Geraint Luff / Signalsmith Audio Ltd.
            Modified by Eliot Deschang, inspired by the original BoxFilter 
            class of enveloppe.h file from SignalSmith dsp library
  ==============================================================================
*/

#pragma once
#include <vector>
#include <algorithm>

template<typename Sample = float>
class BoxSum {
public:
    explicit BoxSum(int maxLength) {
        resize(maxLength);
    }

    void resize(int maxLength) {
        bufferLength = maxLength + 1;
        buffer.resize(bufferLength);
        if (maxLength != 0) buffer.shrink_to_fit();
        reset();
    }

    void reset(Sample value = Sample()) {
        index = 0;
        sum = 0;
        for (size_t i = 0; i < buffer.size(); ++i) {
            buffer[i] = sum;
            sum += value;
        }
        wrapJump = sum;
        sum = 0;
    }

    Sample read(int width) {
        int readIndex = index - width;
        Sample result = sum;
        if (readIndex < 0) {
            result += wrapJump;
            readIndex += bufferLength;
        }
        return result - buffer[readIndex];
    }

    void write(Sample value) {
        ++index;
        if (index == bufferLength) {
            index = 0;
            wrapJump = sum;
            sum = 0;
        }
        sum += value;
        buffer[index] = sum;
    }

    Sample readWrite(Sample value, int width) {
        write(value);
        return read(width);
    }

private:
    int bufferLength, index;
    std::vector<Sample> buffer;
    Sample sum = 0, wrapJump = 0;
    
};

template<typename Sample = float>
class BoxFilter {
public:
    explicit BoxFilter(int maxLength) : boxSum(maxLength) {
        resize(maxLength);
    }

    void resize(int maxLength) {
        _maxLength = maxLength;
        boxSum.resize(maxLength);
        set(maxLength);
    }

    void set(int length) {
        // Only update if the length actually changes
        if (length != _length) {
            _length = length;
            multiplier = Sample(1) / _length;
        }

        /*Original one:
         
        _length = length;
		multiplier = Sample(1)/length;
		if (length > _maxLength) resize(length);
        */
    }

    void reset(Sample fill = Sample()) {
        boxSum.reset(fill);
    }

    Sample operator()(Sample value) {
        return boxSum.readWrite(value, _length) * multiplier;
    }

private:
    BoxSum<Sample> boxSum;
    int _length, _maxLength;
    Sample multiplier;
};
