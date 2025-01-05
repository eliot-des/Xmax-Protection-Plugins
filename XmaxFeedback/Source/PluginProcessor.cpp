/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
XmaxFeedbackAudioProcessor::XmaxFeedbackAudioProcessor()
    : AudioProcessor(
        BusesProperties()
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)
    ),
    params(apvts)
{
    //do nothing
}

XmaxFeedbackAudioProcessor::~XmaxFeedbackAudioProcessor()
{
}

//==============================================================================
const juce::String XmaxFeedbackAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool XmaxFeedbackAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool XmaxFeedbackAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool XmaxFeedbackAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double XmaxFeedbackAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int XmaxFeedbackAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int XmaxFeedbackAudioProcessor::getCurrentProgram()
{
    return 0;
}

void XmaxFeedbackAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String XmaxFeedbackAudioProcessor::getProgramName (int index)
{
    return {};
}

void XmaxFeedbackAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void XmaxFeedbackAudioProcessor::setXuFiltersAndComputation(const LoudspeakerModel& model, float sampleRate)
{
    // Set voltage to displacement conversion
    auto doubleCoeffs = getXUFilterCoefficients(model, sampleRate);
    xuFilterL.setCoefficients(doubleCoeffs.first, doubleCoeffs.second);
    xuFilterR.setCoefficients(doubleCoeffs.first, doubleCoeffs.second);
    xuFilterOutL.setCoefficients(doubleCoeffs.first, doubleCoeffs.second);
    xuFilterOutR.setCoefficients(doubleCoeffs.first, doubleCoeffs.second);

    // Determine which Rms computation function to use based on the Qs value
    if (model.Qs <= Q0) {
        computeRmsComp = computeRmsComp1;
    }
    else {
        gamma = (model.Qs - Q0) / (1 - Cthreshold);
        computeRmsComp = computeRmsComp2;
    }
}

//==============================================================================
void XmaxFeedbackAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    params.prepareToPlay(sampleRate);
    params.reset();


    // Initialize delay line (circular buffer)
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = juce::uint32(samplesPerBlock);
    spec.numChannels = 2;


    int maxDelayInSamples = int(std::ceil(Parameters::maxLookAheadTime * 0.001f * sampleRate));

    delayLineL.setMaximumDelayInSamples(maxDelayInSamples);
    delayLineR.setMaximumDelayInSamples(maxDelayInSamples);
    delayLineL.reset();
    delayLineR.reset();


    //get speaker model to set the coefficients
    currentSpeakerModel = SpeakerModels::modelNames[params.speakerModel];
    lastSpeakerModel = currentSpeakerModel;
    const auto& model = Parameters::speakerModelData.at(currentSpeakerModel);
    setXuFiltersAndComputation(model, sampleRate);

    CmsCompL = model.Cms;
    CmsCompR = model.Cms;

    levelL.reset();
    levelR.reset();
    displacementLevelL.reset();
    displacementLevelR.reset();
}

void XmaxFeedbackAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool XmaxFeedbackAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void XmaxFeedbackAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    //variables for the level and "displacement" meter
    float uMaxL = 0.0f;
    float uMaxR = 0.0f;
    float xMaxL = 0.0f;
    float xMaxR = 0.0f;

    float sampleRate = float(getSampleRate());
    params.update();

    float attackCoeff  = 1 - std::exp(-2.2f / (params.attackTime * 1e-3f * sampleRate));
    float releaseCoeff = 1 - std::exp(-2.2f / (params.releaseTime * 1e-3f * sampleRate));

    currentSpeakerModel = SpeakerModels::modelNames[params.speakerModel];
    const auto& model = Parameters::speakerModelData.at(currentSpeakerModel);

    if (lastSpeakerModel != currentSpeakerModel) {
        setXuFiltersAndComputation(model, sampleRate);
        lastSpeakerModel = currentSpeakerModel;
	}


    float* channelDataL = buffer.getWritePointer(0);
    float* channelDataR = buffer.getWritePointer(1);


    for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {

        params.smoothen();
        int nLookAhead = int(std::ceil(params.lookAheadTime * 1e-3f * sampleRate));

        // take the input signal
        float dryL = channelDataL[sample];
        float dryR = channelDataR[sample];

        // apply the input gain
        uInL = dryL * params.inputGain;
        uInR = dryR * params.inputGain;

        delayLineL.write(uInL);
        delayLineR.write(uInR);

        //displacement estimation
        xL = xuFilterL.processSample(uOutL);
        xR = xuFilterR.processSample(uOutR);
        xL *= params.speakerGain;
        xR *= params.speakerGain;

        //cmsTarget Computation  
        float Xmax = params.thresholdDisplacement * 1e-3f;  
        CmsMin = margin * Xmax * model.Rec / (params.speakerGain * params.inputGain * model.Bl);

        if (std::abs(xL) <= Xmax) CmsTargetL = model.Cms; else CmsTargetL = CmsMin;
        if (std::abs(xR) <= Xmax) CmsTargetR = model.Cms; else CmsTargetR = CmsMin;

        //cmsComp Computation
        CmsCompL = smoothing(CmsTargetL, CmsCompL, attackCoeff, releaseCoeff);
        CmsCompR = smoothing(CmsTargetR, CmsCompR, attackCoeff, releaseCoeff);

        //rmsComp Computation
        RmsCompL = computeRmsComp(CmsCompL, model, Q0, Cthreshold, gamma);
        RmsCompR = computeRmsComp(CmsCompL, model, Q0, Cthreshold, gamma);
        
        //compensation filter update
		auto doubleCoeffsL = getCompFilterCoeffs(model, CmsCompL, RmsCompL, sampleRate);
        auto doubleCoeffsR = getCompFilterCoeffs(model, CmsCompR, RmsCompR, sampleRate);

        compFilterL.setCoefficients(doubleCoeffsL.first, doubleCoeffsL.second);
        compFilterR.setCoefficients(doubleCoeffsR.first, doubleCoeffsR.second);
        compDelayFilterL.setCoefficients(doubleCoeffsL.first, doubleCoeffsL.second);
        compDelayFilterR.setCoefficients(doubleCoeffsR.first, doubleCoeffsR.second);

        //apply the compensation filter on primary path and delayed path
        uOutL = compFilterL.processSample(uInL);
        uOutR = compFilterR.processSample(uInR);

        uOutDelayedL = compDelayFilterL.processSample(delayLineL.read(nLookAhead));
        uOutDelayedR = compDelayFilterR.processSample(delayLineR.read(nLookAhead));


        // output processing - not part of the limiter
        float mixL = params.mix * uOutDelayedL + (1.0f - params.mix) * dryL;
        float mixR = params.mix * uOutDelayedR + (1.0f - params.mix) * dryR;
        float outL = mixL * params.gain;
        float outR = mixR * params.gain;

        channelDataL[sample] = outL;
        channelDataR[sample] = outR;

        // update the level meters
        uMaxL = std::max(uMaxL, std::abs(outL));
        uMaxR = std::max(uMaxR, std::abs(outR));

        // update the displacement meters
        xOutL = xuFilterOutL.processSample(uOutDelayedL);
        xOutR = xuFilterOutR.processSample(uOutDelayedR);
        xMaxL = std::max(xMaxL, std::abs(xOutL * 1e3f * params.speakerGain));
        xMaxR = std::max(xMaxR, std::abs(xOutR * 1e3f * params.speakerGain));
    }

    levelL.updateIfGreater(uMaxL);
    levelR.updateIfGreater(uMaxR);

    displacementLevelL.updateIfGreater(xMaxL);
    displacementLevelR.updateIfGreater(xMaxR);
}

//==============================================================================
bool XmaxFeedbackAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* XmaxFeedbackAudioProcessor::createEditor()
{
    return new XmaxFeedbackAudioProcessorEditor(*this);
}

//==============================================================================
void XmaxFeedbackAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    copyXmlToBinary(*apvts.state.createXml(), destData);
}

void XmaxFeedbackAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
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
    return new XmaxFeedbackAudioProcessor();
}
