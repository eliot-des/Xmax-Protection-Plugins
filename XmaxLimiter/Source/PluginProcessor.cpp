/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
XmaxLimiterAudioProcessor::XmaxLimiterAudioProcessor()
     : AudioProcessor (
         BusesProperties()
            .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
            .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
    ),
    params(apvts)
{
    //do nothing
}

XmaxLimiterAudioProcessor::~XmaxLimiterAudioProcessor()
{
}

//==============================================================================
const juce::String XmaxLimiterAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool XmaxLimiterAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool XmaxLimiterAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool XmaxLimiterAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double XmaxLimiterAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int XmaxLimiterAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int XmaxLimiterAudioProcessor::getCurrentProgram()
{
    return 0;
}

void XmaxLimiterAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String XmaxLimiterAudioProcessor::getProgramName (int index)
{
    return {};
}

void XmaxLimiterAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}
//==============================================================================

void XmaxLimiterAudioProcessor::setFiltersCoeffs(const LoudspeakerModel& model, float sampleRate)
{
    auto doubleCoeffs = getXUFilterCoefficients(model, sampleRate, 0.95f);
    std::array<float, 3> b_xu = doubleCoeffs.first;
    std::array<float, 3> a_xu = doubleCoeffs.second;

    xuFilterL.setCoefficients(b_xu, a_xu);
    xuFilterR.setCoefficients(b_xu, a_xu);

    auto b_ux = a_xu;
    auto a_ux = b_xu;
    //normalize the coefficients for the displacement to tension filter

    float norm = a_ux[0];
    for (auto& coef : a_ux) coef /= norm;
    for (auto& coef : b_ux) coef /= norm;

    uxFilterL.setCoefficients(b_ux, a_ux);
    uxFilterR.setCoefficients(b_ux, a_ux);
}

//==============================================================================
void XmaxLimiterAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
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

    levelL.reset();
    levelR.reset();
    displacementLevelL.reset();
    displacementLevelR.reset();
}

void XmaxLimiterAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool XmaxLimiterAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void XmaxLimiterAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& /*midiMessages*/)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());


    float maxL = 0.0f;
    float maxR = 0.0f;
    float maxDispL = 0.0f;
    float maxDispR = 0.0f;

    float sampleRate = float(getSampleRate());

    params.update();

    
    float releaseCoeff = 1 - std::exp(-2.2f / (float(sampleRate) * params.releaseTime * 0.001f));

    currentSpeakerModel = SpeakerModels::modelNames[params.speakerModel];
    const auto& model = Parameters::speakerModelData.at(currentSpeakerModel);

    if (currentSpeakerModel != lastSpeakerModel) {
        setFiltersCoeffs(model, sampleRate);
        lastSpeakerModel = currentSpeakerModel;
    }

    float* channelDataL = buffer.getWritePointer(0);
    float* channelDataR = buffer.getWritePointer(1);

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

        
        if (params.limiterMode == 1) { //displacement mode.
            varL = xuFilterL.processSample(inputAmpL);
            varR = xuFilterR.processSample(inputAmpR);

            threshold = params.thresholdDisplacement * 1e-3f; //convert in m
            gain = params.speakerGain;
        }
        else { //level mode 
		    varL = inputAmpL; //keep the tension signal
		    varR = inputAmpR;

            threshold = params.thresholdTension;
            gain = 1.0f;
        }

        
        delayLineL.write(varL);
        delayLineR.write(varR);

        knee = params.knee;
        
        gcL = computeGain(std::abs(varL) * gain, threshold, knee);
        gcR = computeGain(std::abs(varR) * gain, threshold, knee);
        
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

        //Apply the averaging filter to the minimum filter output.
        gL = rectFilterL(cL);
        gR = rectFilterR(cR);

        float limL = gL * delayLineL.read(nAttack);
        float limR = gR * delayLineR.read(nAttack);
        
        //convert the displacement signal back to a tension signal if in displacement mode

        if (params.limiterMode == 1) {

            maxDispL = std::max(maxDispL, std::abs(limL * params.speakerGain * 1e3f));
            maxDispR = std::max(maxDispR, std::abs(limR * params.speakerGain * 1e3f));

			wetL = uxFilterL.processSample(limL);
			wetR = uxFilterR.processSample(limR);
		}
        else {
            wetL = limL;
			wetR = limR;
        }
        
        // output processing - not part of the limiter
        float mixL = params.mix * wetL + (1.0f - params.mix) * dryL;
        float mixR = params.mix * wetR + (1.0f - params.mix) * dryR;

        float outL = mixL * params.gain;
        float outR = mixR * params.gain;


        channelDataL[sample] = outL;
        channelDataR[sample] = outR;

        maxL = std::max(maxL, std::abs(outL));
        maxR = std::max(maxR, std::abs(outR));
    }

    levelL.updateIfGreater(maxL);
    levelR.updateIfGreater(maxR);

    displacementLevelL.updateIfGreater(maxDispL);
    displacementLevelR.updateIfGreater(maxDispR);


}

//==============================================================================
bool XmaxLimiterAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* XmaxLimiterAudioProcessor::createEditor()
{
    return new XmaxLimiterAudioProcessorEditor(*this);
}

//==============================================================================
void XmaxLimiterAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    copyXmlToBinary(*apvts.state.createXml(), destData);
}

void XmaxLimiterAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
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
    return new XmaxLimiterAudioProcessor();

}
