/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
XmaxLimiterAudioProcessorEditor::XmaxLimiterAudioProcessorEditor (XmaxLimiterAudioProcessor& p)
    : AudioProcessorEditor (&p), 
    audioProcessor (p), 
    meter(p.levelL, p.levelR), 
    displacementMeter(p.displacementLevelL, p.displacementLevelR, p.params.thresholdDisplacement)
{

    inputGroup.setText("Input");
    inputGroup.setTextLabelPosition(juce::Justification::horizontallyCentred);
    inputGroup.addAndMakeVisible(inputGainKnob);
    stereoButton.setButtonText("Stereo");
    stereoButton.setClickingTogglesState(true);
    stereoButton.setBounds(0, 0, 70, 27);
    stereoButton.onClick = [this]() {
        if (stereoButton.getToggleState()) {stereoButton.setButtonText("Stereo");}
        else { stereoButton.setButtonText("Merged");}
    };
    stereoButton.setLookAndFeel(ButtonLookAndFeel::get());
    inputGroup.addAndMakeVisible(stereoButton);
    addAndMakeVisible(inputGroup);


    speakerGroup.setText("Speaker");
    speakerGroup.setTextLabelPosition(juce::Justification::horizontallyCentred);
    speakerGroup.addAndMakeVisible(speakerGainKnob);
    speakerModelComboBox.setBounds(0, 0, 70, 27);
    speakerModelComboBox.addItemList(SpeakerModels::modelNames, 1);
    speakerModelComboBox.setSelectedItemIndex(audioProcessor.params.speakerModelParam->getIndex(), juce::dontSendNotification);
    speakerModelComboBox.setLookAndFeel(ComboBoxLookAndFeel::get());
    speakerGroup.addAndMakeVisible(speakerModelComboBox);
    speakerComboBoxLabel.setText("Driver Model", juce::dontSendNotification);
    speakerComboBoxLabel.setColour(juce::Label::textColourId, Colors::Button::text);
    speakerComboBoxLabel.setJustificationType(juce::Justification::centred);
    speakerComboBoxLabel.attachToComponent(&speakerModelComboBox, false);
    speakerGroup.addAndMakeVisible(speakerComboBoxLabel);
    speakerGroup.addAndMakeVisible(displacementMeter);
    addAndMakeVisible(speakerGroup);


    envelopeGroup.setText("Envelope");
    envelopeGroup.setTextLabelPosition(juce::Justification::horizontallyCentred);
    envelopeGroup.addAndMakeVisible(attackTimeKnob);
    envelopeGroup.addAndMakeVisible(holdTimeKnob);
    envelopeGroup.addAndMakeVisible(releaseTimeKnob);
    addAndMakeVisible(envelopeGroup);

    gainComputerGroup.setText("Gain Comp.");
    gainComputerGroup.setTextLabelPosition(juce::Justification::horizontallyCentred);
    gainComputerGroup.addAndMakeVisible(thresholdTensionKnob);
    gainComputerGroup.addAndMakeVisible(thresholdDisplacementKnob);
    gainComputerGroup.addAndMakeVisible(kneeKnob);
    
    limiterModeComboBox.setBounds(0, 0, 70, 27);
    limiterModeComboBox.addItemList(LimiterModes::modeNames, 1);
    limiterModeComboBox.setSelectedItemIndex(audioProcessor.params.limiterModeParam->getIndex(), juce::dontSendNotification);
    limiterModeComboBox.setLookAndFeel(ComboBoxLookAndFeel::get());
    gainComputerGroup.addAndMakeVisible(limiterModeComboBox);
    
    limiterComboBoxLabel.setText("Mode", juce::dontSendNotification);
    limiterComboBoxLabel.setColour(juce::Label::textColourId, Colors::Button::text);
    limiterComboBoxLabel.setJustificationType(juce::Justification::centred);
    limiterComboBoxLabel.attachToComponent(&limiterModeComboBox, false);
    gainComputerGroup.addAndMakeVisible(limiterComboBoxLabel);
    addAndMakeVisible(gainComputerGroup);

    outputGroup.setText("Output");
    outputGroup.setTextLabelPosition(juce::Justification::horizontallyCentred);
    outputGroup.addAndMakeVisible(gainKnob);
    outputGroup.addAndMakeVisible(mixKnob);
    outputGroup.addAndMakeVisible(meter);
    addAndMakeVisible(outputGroup);

    setSize (740, 350);
    setLookAndFeel(&mainLF);

    updateThresholdKnobs(audioProcessor.params.limiterModeParam->getIndex() == 1);
    audioProcessor.params.limiterModeParam->addListener(this);


    thresholdDisplacementKnob.slider.onValueChange = [this]() {
        displacementMeter.setThresholdDisplacement(thresholdDisplacementKnob.slider.getValue());
        };

    websiteLinkButton.setFont(juce::Font(15.0f, 0), false, juce::Justification::centredRight);
    websiteLinkButton.setColour(juce::HyperlinkButton::textColourId, Colors::background);
    addAndMakeVisible(websiteLinkButton);
}

XmaxLimiterAudioProcessorEditor::~XmaxLimiterAudioProcessorEditor()
{
    audioProcessor.params.limiterModeParam->removeListener(this);
    setLookAndFeel(nullptr);
}


//==============================================================================
void XmaxLimiterAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    //g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
    g.fillAll(Colors::background);
    g.setColour(Colors::header);

    g.fillRect(0, 0, getWidth(), 50);
    //Add big text in the rectangle, change font to bold
    g.setFont(juce::Font(40.0f, juce::Font::bold));
    g.setColour(Colors::background);
    g.drawText("Xmax Limiter", 20, 0, getWidth(), 50, juce::Justification::centredLeft);
    g.setFont(Fonts::getFont(15.0f));
    g.drawText("Le Mans University", 20, 10, getWidth() - 40, 15, juce::Justification::centredRight);
    //g.drawText("Eliot Deschang, Florian Marie", 20, 25, getWidth() - 40, 15, juce::Justification::centredRight);
}

void XmaxLimiterAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    auto area = getLocalBounds();
    int groupWidth = 120;
    int groupHeight = 150;
    int groupMargin = 5;

    area.removeFromTop(50);

    inputGroup.setBounds(area.removeFromLeft(groupWidth).reduced(groupMargin));
    speakerGroup.setBounds(area.removeFromLeft(groupWidth + 40).reduced(groupMargin));
    outputGroup.setBounds(area.removeFromRight(groupWidth + 40).reduced(groupMargin));
    envelopeGroup.setBounds(area.removeFromTop(groupHeight).reduced(groupMargin));
    gainComputerGroup.setBounds(area.reduced(groupMargin));

    // Position the knobs inside the groups
    inputGainKnob.setTopLeftPosition(20, 20);
    stereoButton.setTopLeftPosition(20, inputGainKnob.getBottom() + 70);

    speakerGainKnob.setTopLeftPosition(20, 20);
    speakerModelComboBox.setTopLeftPosition(20, speakerGainKnob.getBottom() + 70);
    displacementMeter.setBounds(outputGroup.getWidth() - 45, 30, 35, speakerModelComboBox.getBottom() + 3);

    attackTimeKnob.setTopLeftPosition(20, 15);
    holdTimeKnob.setTopLeftPosition(attackTimeKnob.getRight() + 20, 15);
    releaseTimeKnob.setTopLeftPosition(holdTimeKnob.getRight() + 20, 15);

    limiterModeComboBox.setTopLeftPosition(20, 60);
    thresholdTensionKnob.setTopLeftPosition(limiterModeComboBox.getRight() + 20, 15);
    thresholdDisplacementKnob.setTopLeftPosition(thresholdTensionKnob.getX(), thresholdTensionKnob.getY());
    kneeKnob.setTopLeftPosition(thresholdTensionKnob.getRight() + 20, 15);

    mixKnob.setTopLeftPosition(20, 20);
    gainKnob.setTopLeftPosition(mixKnob.getX(), mixKnob.getBottom() + 20);
    meter.setBounds(outputGroup.getWidth() - 45, 30, 30, gainKnob.getBottom() - 30);


    websiteLinkButton.setBounds(520, 25, 200, 15);
}


void XmaxLimiterAudioProcessorEditor::parameterValueChanged(int, float value)
{
    if (juce::MessageManager::getInstance()->isThisTheMessageThread()) {
        updateThresholdKnobs(value != 0.0f);
    }
    else {
        juce::MessageManager::callAsync([this, value]
            {
                updateThresholdKnobs(value != 0.0f);
            });
    }
}


void XmaxLimiterAudioProcessorEditor::updateThresholdKnobs(bool displacement)
{
        thresholdDisplacementKnob.setVisible(displacement);
        thresholdTensionKnob.setVisible(!displacement);
}