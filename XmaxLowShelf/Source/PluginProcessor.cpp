/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
XmaxLowShelfAudioProcessor::XmaxLowShelfAudioProcessor()
    : AudioProcessor(
        BusesProperties()
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)
    ),
    params(apvts)
{
    //do nothing
}

XmaxLowShelfAudioProcessor::~XmaxLowShelfAudioProcessor()
{
}

//==============================================================================
const juce::String XmaxLowShelfAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool XmaxLowShelfAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool XmaxLowShelfAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool XmaxLowShelfAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double XmaxLowShelfAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int XmaxLowShelfAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int XmaxLowShelfAudioProcessor::getCurrentProgram()
{
    return 0;
}

void XmaxLowShelfAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String XmaxLowShelfAudioProcessor::getProgramName (int index)
{
    return {};
}

void XmaxLowShelfAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}
//==============================================================================

void XmaxLowShelfAudioProcessor::setFiltersCoeffs(const LoudspeakerModel& model, float sampleRate)
{
    auto doubleCoeffs = getXUFilterCoefficients(model, sampleRate);
    std::array<float, 3> b_xu = doubleCoeffs.first;
    std::array<float, 3> a_xu = doubleCoeffs.second;

    xuFilterInL.setCoefficients(b_xu, a_xu);
    xuFilterInR.setCoefficients(b_xu, a_xu);

    xuFilterOutL.setCoefficients(b_xu, a_xu);
    xuFilterOutR.setCoefficients(b_xu, a_xu);

    //set lowShelf filter coefficients
    auto doubleShelfCoeffs = getLowShelfCoefficients(model.fs, Q, shelfGainL, float(sampleRate));
    std::array<float, 3> b_shelf = doubleShelfCoeffs.first;
    std::array<float, 3> a_shelf = doubleShelfCoeffs.second;

    lowShelfFilterL.setCoefficients(b_shelf, a_shelf);
    lowShelfFilterR.setCoefficients(b_shelf, a_shelf);
}

//==============================================================================
void XmaxLowShelfAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    params.prepareToPlay(sampleRate);
    params.reset();


    // Initialize delay line (circular buffer)
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = juce::uint32(samplesPerBlock);
    spec.numChannels = 2;


    float maxDelayTimeSignal = Parameters::maxAttackTime;
    float maxDelayTimeMinFilter = Parameters::maxAttackTime + Parameters::maxHoldTime;
    double numSamplesMinFilter = maxDelayTimeMinFilter * 0.001f * sampleRate;

    double numSamplesSignal = maxDelayTimeSignal * 0.001f * sampleRate;

    int maxDelayInSamplesMinFilter = int(std::ceil(numSamplesMinFilter));
    int maxDelayInSamplesSignal = int(std::ceil(numSamplesSignal));


    minFilterL = MinFilter<float>(maxDelayInSamplesMinFilter);
    minFilterR = MinFilter<float>(maxDelayInSamplesMinFilter);

    delayLineL.setMaximumDelayInSamples(maxDelayInSamplesSignal);
    delayLineR.setMaximumDelayInSamples(maxDelayInSamplesSignal);
    delayLineL.reset();
    delayLineR.reset();

    rectFilterL.resize(maxDelayInSamplesSignal);
    rectFilterR.resize(maxDelayInSamplesSignal);
    rectFilterL.reset(1);
    rectFilterR.reset(1);

    //get speaker model to set the coefficients
    currentSpeakerModel = SpeakerModels::modelNames[params.speakerModel];
    lastSpeakerModel = currentSpeakerModel;
    const auto& model = Parameters::speakerModelData.at(currentSpeakerModel);
    setFiltersCoeffs(model, sampleRate);

    filterProcessorL = [](float input, float gain) { return input; };
    filterProcessorR = filterProcessorL;

    levelL.reset();
    levelR.reset();
    displacementLevelL.reset();
    displacementLevelR.reset();
}

void XmaxLowShelfAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool XmaxLowShelfAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void XmaxLowShelfAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());


    float maxL = 0.0f;
    float maxR = 0.0f;
    float maxDispL = 0.0f;
    float maxDispR = 0.0f;

    float* channelDataL = buffer.getWritePointer(0);
    float* channelDataR = buffer.getWritePointer(1);

    float sampleRate = float(getSampleRate());

    params.update();


    float releaseCoeff = 1 - std::exp(-2.2f / (sampleRate * params.releaseTime * 0.001f));

    currentSpeakerModel = SpeakerModels::modelNames[params.speakerModel];
    const auto& model = Parameters::speakerModelData.at(currentSpeakerModel);

    if (currentSpeakerModel != lastSpeakerModel) {
        setFiltersCoeffs(model, sampleRate);
        lastSpeakerModel = currentSpeakerModel;
    }

    if (params.filterMode == 0) { // Low-shelf filter mode
        filterProcessorL = [this](float input, float gain) -> float {
            if (shelfGainL != lastShelfGainL) {
                auto shelfCoeffsL = getLowShelfCoefficients(fc, Q, shelfGainL, getSampleRate());
                lowShelfFilterL.setCoefficients(shelfCoeffsL.first, shelfCoeffsL.second);
                lastShelfGainL = shelfGainL;
            }
            return lowShelfFilterL.processSample(input);
            };
        filterProcessorR = [this](float input, float gain) -> float {
            if (shelfGainR != lastShelfGainR) {
                auto shelfCoeffsR = getLowShelfCoefficients(fc, Q, shelfGainR, getSampleRate());
                lowShelfFilterR.setCoefficients(shelfCoeffsR.first, shelfCoeffsR.second);
                lastShelfGainR = shelfGainR;
            }
            return lowShelfFilterR.processSample(input);
            };
    }
    else { // Simple gain mode
        filterProcessorL = [](float input, float gain) -> float {
            return gain * input;
            };
        filterProcessorR = filterProcessorL;
    }



    for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
        params.smoothen();

        int nAttack = int(std::ceil(params.attackTime * 1e-3f * sampleRate));
        int nAttackHold = int(std::ceil((params.attackTime + params.holdTime) * 1e-3f * sampleRate));

        minFilterL.set(nAttackHold);
        minFilterR.set(nAttackHold);
        rectFilterL.set(nAttack);
        rectFilterR.set(nAttack);

        // take the input signal
        float dryL = channelDataL[sample];
        float dryR = channelDataR[sample];

        // apply the input gain
        float inputAmpL = dryL * params.inputGain;
        float inputAmpR = dryR * params.inputGain;

        delayLineL.write(inputAmpL);
        delayLineR.write(inputAmpR);

        xInL = xuFilterInL.processSample(inputAmpL); 
        xInR = xuFilterInR.processSample(inputAmpR);

        threshold = params.thresholdDisplacement * 1e-3f; //convert in m
        knee = params.knee;

        gain = params.speakerGain;

        gcL = computeGain(std::abs(xInL) * gain, threshold, knee);
        gcR = computeGain(std::abs(xInR) * gain, threshold, knee);

        //store the gain computer function  output in the circular buffers for the minimum filter
        minFilterL.add(gcL);
        minFilterR.add(gcR);
        float minGainL = minFilterL.getMinimum();
        float minGainR = minFilterR.getMinimum();

        //apply exponential release to the minimum filter output
        cL = std::min(minGainL, (1.0f - releaseCoeff) * cL + releaseCoeff * minGainL);
        cR = std::min(minGainR, (1.0f - releaseCoeff) * cR + releaseCoeff * minGainR);

        // I would say that because we use an 1 pole filter for the release, the gain will never reach 1.0 anymore, because when the release is applied
        // the gain will tends toward 1 during release phase, but will never really reach it, so the averaging filter will never reach 1.0. 
        // To prevent this:
        if (cL > 0.999f) cL = 1.0f;
        if (cR > 0.999f) cR = 1.0f;

        //Apply the averaging filter to the exponential release output.
        gL = rectFilterL(cL);
        gR = rectFilterR(cR);

        //convert the linear gain to dB
        shelfGainL = 20.0f * std::log10(gL);
        shelfGainR = 20.0f * std::log10(gR);

        //this big if loop could be simplified by using a function that would return the output of the filter depending on the mode...
        // Use pre-set filter logic
        wetL = filterProcessorL(delayLineL.read(nAttack), gL);
        wetR = filterProcessorR(delayLineR.read(nAttack), gR);

        // output processing - not part of the limiter
        float mixL = params.mix * wetL + (1.0f - params.mix) * dryL;
        float mixR = params.mix * wetR + (1.0f - params.mix) * dryR;

        float outL = mixL * params.gain;
        float outR = mixR * params.gain;

        channelDataL[sample] = outL;
        channelDataR[sample] = outR;

        //convert the limited signal to displacement to check the displacement level
        xOutL = xuFilterOutL.processSample(wetL);
        xOutR = xuFilterOutR.processSample(wetR);

        //output displacement level to display on the displacement level meter
        maxDispL = std::max(maxDispL, std::abs(xOutL * 1e3f * params.speakerGain));
        maxDispR = std::max(maxDispR, std::abs(xOutR * 1e3f * params.speakerGain));

        maxL = std::max(maxL, std::abs(outL));
        maxR = std::max(maxR, std::abs(outR));
    }

    levelL.updateIfGreater(maxL);
    levelR.updateIfGreater(maxR);

    displacementLevelL.updateIfGreater(maxDispL);
    displacementLevelR.updateIfGreater(maxDispR);
}

//==============================================================================
bool XmaxLowShelfAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* XmaxLowShelfAudioProcessor::createEditor()
{
    return new XmaxLowShelfAudioProcessorEditor(*this);
}

//==============================================================================
void XmaxLowShelfAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    copyXmlToBinary(*apvts.state.createXml(), destData);
}

void XmaxLowShelfAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));
    if (xml.get() != nullptr && xml->hasTagName(apvts.state.getType()))
    {
        apvts.replaceState(juce::ValueTree::fromXml(*xml));
    }
}
//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new XmaxLowShelfAudioProcessor();
}
