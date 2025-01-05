/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
XmaxFeedbackAudioProcessorEditor::XmaxFeedbackAudioProcessorEditor (XmaxFeedbackAudioProcessor& p)
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
        if (stereoButton.getToggleState()) { stereoButton.setButtonText("Stereo"); }
        else { stereoButton.setButtonText("Merged"); }
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
    speakerComboBoxLabel.setText("Driver Model", juce::dontSendNotification);
    speakerComboBoxLabel.setColour(juce::Label::textColourId, Colors::Button::text);
    speakerComboBoxLabel.setJustificationType(juce::Justification::centred);
    speakerComboBoxLabel.attachToComponent(&speakerModelComboBox, false);
    speakerGroup.addAndMakeVisible(speakerModelComboBox);
    speakerGroup.addAndMakeVisible(speakerComboBoxLabel);
    speakerGroup.addAndMakeVisible(displacementMeter);
    addAndMakeVisible(speakerGroup);


    sideChainGroup.setText("Control Chain");
    sideChainGroup.setTextLabelPosition(juce::Justification::horizontallyCentred);
    sideChainGroup.addAndMakeVisible(attackTimeKnob);
    sideChainGroup.addAndMakeVisible(releaseTimeKnob);
    sideChainGroup.addAndMakeVisible(thresholdDisplacementKnob);
    sideChainGroup.addAndMakeVisible(lookAheadTimeKnob);
    addAndMakeVisible(sideChainGroup);

    outputGroup.setText("Output");
    outputGroup.setTextLabelPosition(juce::Justification::horizontallyCentred);
    outputGroup.addAndMakeVisible(gainKnob);
    outputGroup.addAndMakeVisible(mixKnob);
    outputGroup.addAndMakeVisible(meter);
    addAndMakeVisible(outputGroup);

    setSize(650, 340);
    setLookAndFeel(&mainLF);


    thresholdDisplacementKnob.slider.onValueChange = [this]() {
        displacementMeter.setThresholdDisplacement(thresholdDisplacementKnob.slider.getValue());
        };

    websiteLinkButton.setFont(juce::Font(15.0f, 0), false, juce::Justification::centredRight);
    websiteLinkButton.setColour(juce::HyperlinkButton::textColourId, Colors::background);
    addAndMakeVisible(websiteLinkButton);
}

XmaxFeedbackAudioProcessorEditor::~XmaxFeedbackAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

//==============================================================================
void XmaxFeedbackAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll(Colors::background);
    g.setColour(Colors::header);

    g.fillRect(0, 0, getWidth(), 50);
    //Add big text in the rectangle, change font to bold
    g.setFont(juce::Font(40.0f, juce::Font::bold));
    g.setColour(Colors::background);
    g.drawText("Xmax Feedback", 20, 0, getWidth(), 50, juce::Justification::centredLeft);
    g.setFont(Fonts::getFont(15.0f));
    g.drawText("Le Mans University", 20, 10, getWidth() - 40, 15, juce::Justification::centredRight);
    //g.drawText("Eliot Deschang, Florian Marie", 20, 25, getWidth()-40, 15, juce::Justification::centredRight);
}

void XmaxFeedbackAudioProcessorEditor::resized()
{
    auto area = getLocalBounds();
    int groupWidth = 120;
    int groupHeight = 150;
    int groupMargin = 5;

    area.removeFromTop(50);

    inputGroup.setBounds(area.removeFromLeft(groupWidth).reduced(groupMargin));
    speakerGroup.setBounds(area.removeFromLeft(groupWidth + 40).reduced(groupMargin));
    outputGroup.setBounds(area.removeFromRight(groupWidth + 40).reduced(groupMargin));
    sideChainGroup.setBounds(area.reduced(groupMargin));

    // Position the knobs inside the groups
    inputGainKnob.setTopLeftPosition(20, 20);
    stereoButton.setTopLeftPosition(20, inputGainKnob.getBottom() + 70);

    speakerGainKnob.setTopLeftPosition(20, 20);
    speakerModelComboBox.setTopLeftPosition(20, speakerGainKnob.getBottom() + 70);
    displacementMeter.setBounds(outputGroup.getWidth() - 45, 25, 35, speakerModelComboBox.getBottom() + 3);

    attackTimeKnob.setTopLeftPosition(20, 20);
    releaseTimeKnob.setTopLeftPosition(attackTimeKnob.getRight() + 20, 20);

    thresholdDisplacementKnob.setTopLeftPosition(attackTimeKnob.getX(), attackTimeKnob.getBottom() + 20);
    lookAheadTimeKnob.setTopLeftPosition(thresholdDisplacementKnob.getRight() + 20, thresholdDisplacementKnob.getY());

    mixKnob.setTopLeftPosition(20, 20);
    gainKnob.setTopLeftPosition(mixKnob.getX(), mixKnob.getBottom() + 20);
    meter.setBounds(outputGroup.getWidth() - 45, 25, 30, gainKnob.getBottom() - 30);


    websiteLinkButton.setBounds(430, 25, 200, 15);
}
