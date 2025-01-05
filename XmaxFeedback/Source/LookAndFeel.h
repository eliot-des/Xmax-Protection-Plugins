/*
  ==============================================================================

    LookAndFeel.h
    Created: 2 Jan 2025 12:28:56pm
    Author:  eliot

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

namespace Colors
{
    const juce::Colour background{ 245, 240, 235 };
    const juce::Colour header{ 40, 40, 40 };

    namespace Knob
    {
        const juce::Colour trackBackground{ 205, 200, 195 };
        const juce::Colour trackActive{ 40, 180, 99 };
        const juce::Colour outline{ 255, 250, 245 };
        const juce::Colour gradientTop{ 250, 245, 240 };
        const juce::Colour gradientBottom{ 240, 235, 230 };
        const juce::Colour dial{ 100, 100, 100 };
        const juce::Colour dropShadow{ 195, 190, 185 };
        const juce::Colour label{ 80, 80, 80 };
        const juce::Colour textBoxBackground{ 80, 80, 80 };
        const juce::Colour value{ 240, 240, 240 };
        const juce::Colour caret{ 255, 255, 255 };
    }

    namespace Group
    {
        const juce::Colour label{ 160, 155, 150 };
        const juce::Colour outline{ 235, 230, 225 };
    }

    namespace Button
    {
        const juce::Colour text{ 80, 80, 80 };
        const juce::Colour textToggled{ 40, 40, 40 };
        const juce::Colour background{ 245, 240, 235 };
        const juce::Colour backgroundToggled{ 255, 250, 245 };
        const juce::Colour outline{ 235, 230, 225 };
    }

    namespace ComboBox
    {
        const juce::Colour background{ 255, 250, 245 };
        const juce::Colour outline{ 235, 230, 225 };
        const juce::Colour outlineFocused{ 80, 80, 80 };
        const juce::Colour text{ 80, 80, 80 };
        const juce::Colour button{ 80, 80, 80 };
        const juce::Colour arrow{ 40, 40, 40 };
        const juce::Colour labelText{ 160, 155, 150 };
        const juce::Colour popupBackground{ 255, 250, 245 };
        const juce::Colour highlightedTextBox{ 80, 80, 80 };
    }

    namespace LevelMeter
    {
        const juce::Colour background{ 245, 240, 235 };
        const juce::Colour tickLine{ 200, 200, 200 };
        const juce::Colour tickLabel{ 80, 80, 80 };
        const juce::Colour tooLoud{ 226, 74, 81 };
        const juce::Colour levelOK{ 65, 206, 88 };
    }
    namespace DisplacementMeter
    {
        const juce::Colour background{ 245, 240, 235 };
        const juce::Colour tickLine{ 200, 200, 200 };
        const juce::Colour thresholdLine{ 100, 100, 100 };
        const juce::Colour tickLabel{ 80, 80, 80 };
        const juce::Colour level{ 53, 156, 255 };
    }
}


class Fonts
{
public:
    Fonts() = delete;

    static juce::Font getFont(float height = 16.0f);

private:
    static const juce::Typeface::Ptr typeface;
};

class RotaryKnobLookAndFeel : public juce::LookAndFeel_V4
{
public:
    RotaryKnobLookAndFeel();

    static RotaryKnobLookAndFeel* get()
    {
        static RotaryKnobLookAndFeel instance;
        return &instance;
    }

    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
        float sliderPos, float rotaryStartAngle,
        float rotaryEndAngle, juce::Slider& slider) override;

    juce::Font getLabelFont(juce::Label&) override;
    juce::Label* createSliderTextBox(juce::Slider&) override;

    void drawTextEditorOutline(juce::Graphics&, int, int, juce::TextEditor&) override { }
    void fillTextEditorBackground(juce::Graphics&, int width, int height, juce::TextEditor&) override;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RotaryKnobLookAndFeel)

        juce::DropShadow dropShadow{ Colors::Knob::dropShadow, 6, { 0, 3 } };
};

class MainLookAndFeel : public juce::LookAndFeel_V4
{
public:
    MainLookAndFeel();

    juce::Font getLabelFont(juce::Label&) override;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainLookAndFeel)
};

class ButtonLookAndFeel : public juce::LookAndFeel_V4
{
public:
    ButtonLookAndFeel();

    static ButtonLookAndFeel* get()
    {
        static ButtonLookAndFeel instance;
        return &instance;
    }

    void drawButtonBackground(juce::Graphics& g, juce::Button& button,
        const juce::Colour& backgroundColour,
        bool shouldDrawButtonAsHighlighted,
        bool shouldDrawButtonAsDown) override;

    void drawButtonText(juce::Graphics& g, juce::TextButton& button,
        bool shouldDrawButtonAsHighlighted,
        bool shouldDrawButtonAsDown) override;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ButtonLookAndFeel)
};


class ComboBoxLookAndFeel : public juce::LookAndFeel_V4
{
public:
    ComboBoxLookAndFeel();

    static ComboBoxLookAndFeel* get()
    {
        static ComboBoxLookAndFeel instance;
        return &instance;
    }

    void ComboBoxLookAndFeel::drawPopupMenuBackground(juce::Graphics& g, int width, int height) override;

    void drawPopupMenuItem(juce::Graphics&, const juce::Rectangle<int>& area,
        bool isSeparator, bool isActive, bool isHighlighted, bool isTicked, bool hasSubMenu,
        const juce::String& text, const juce::String& shortcutKeyText,
        const juce::Drawable* icon, const juce::Colour* textColour) override;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ComboBoxLookAndFeel)
};
