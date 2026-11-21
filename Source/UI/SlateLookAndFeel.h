#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

namespace yeahwaah
{

class SlateLookAndFeel : public juce::LookAndFeel_V4
{
public:
    SlateLookAndFeel();
    ~SlateLookAndFeel() override = default;

    // Custom styling overrides
    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                          float sliderPosProportional, float rotaryStartAngle,
                          float rotaryEndAngle, juce::Slider& slider) override;

    void drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
                          float sliderPos, float minSliderPos, float maxSliderPos,
                          const juce::Slider::SliderStyle, juce::Slider& slider) override;

    void drawButtonBackground(juce::Graphics& g, juce::Button& button,
                              const juce::Colour& backgroundColour,
                              bool shouldDrawButtonAsHighlighted,
                              bool shouldDrawButtonAsDown) override;

    void drawComboBox(juce::Graphics& g, int width, int height, bool isButtonDown,
                      int buttonX, int buttonY, int buttonW, int buttonH,
                      juce::ComboBox& box) override;

    void drawPopupMenuBackground(juce::Graphics& g, int width, int height) override;

    // Slate Color Palette
    static const juce::Colour bgDark;
    static const juce::Colour bgPanel;
    static const juce::Colour bgControl;
    static const juce::Colour cyanGlow;
    static const juce::Colour amberGlow;
    static const juce::Colour slateBorder;
    static const juce::Colour textLight;
    static const juce::Colour textDim;
    static const juce::Colour soloYellow;
    static const juce::Colour muteRed;
};

} // namespace yeahwaah
