#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

namespace yeahwaah
{

class SlateLookAndFeel : public juce::LookAndFeel_V4
{
public:
    SlateLookAndFeel();
    ~SlateLookAndFeel() override = default;

    // Custom styling overrides for SSL Console + Slate Trigger 2
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

    void drawButtonText(juce::Graphics& g, juce::TextButton& button,
                        bool shouldDrawButtonAsHighlighted,
                        bool shouldDrawButtonAsDown) override;

    void drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
                          bool shouldDrawButtonAsHighlighted,
                          bool shouldDrawButtonAsDown) override;

    void drawComboBox(juce::Graphics& g, int width, int height, bool isButtonDown,
                      int buttonX, int buttonY, int buttonW, int buttonH,
                      juce::ComboBox& box) override;

    void drawPopupMenuBackground(juce::Graphics& g, int width, int height) override;

    void drawPopupMenuItem(juce::Graphics& g, const juce::Rectangle<int>& area,
                           bool isSeparator, bool isActive, bool isHighlighted,
                           bool isTicked, bool hasSubMenu,
                           const juce::String& text, const juce::String& shortcutKeyText,
                           const juce::Drawable* icon, const juce::Colour* textColour) override;

    juce::Font getLabelFont(juce::Label& label) override;

    // Steven Slate Drums Trigger 2 & SSL Console Color Palette
    static const juce::Colour bgDark;
    static const juce::Colour bgPanel;
    static const juce::Colour bgControl;
    static const juce::Colour cyanGlow;
    static const juce::Colour amberGlow;
    static const juce::Colour slateBorder;
    static const juce::Colour slateHighlight;
    static const juce::Colour textLight;
    static const juce::Colour textDim;
    static const juce::Colour soloYellow;
    static const juce::Colour muteRed;
    static const juce::Colour phaseBlue;

    // SSL Console Channel Colors
    static const juce::Colour sslBlue;
    static const juce::Colour sslGreen;
    static const juce::Colour sslYellow;
    static const juce::Colour sslRed;
    static const juce::Colour sslAmber;
    static const juce::Colour sslCyan;
    static const juce::Colour sslWhite;
    static const juce::Colour sslDark;
    static const juce::Colour sslPanel;

    // Channel color accents (Slots 1 to 6)
    static juce::Colour getSlotColor(int slotIndex);
};

} // namespace yeahwaah
