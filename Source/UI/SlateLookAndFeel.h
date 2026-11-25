#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

namespace yeahwaah
{

// Vintage studio-hardware look: dark brushed metal, cream engraving, amber illumination.
class SlateLookAndFeel : public juce::LookAndFeel_V4
{
public:
    SlateLookAndFeel();
    ~SlateLookAndFeel() override = default;

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

    void positionComboBoxText(juce::ComboBox& box, juce::Label& label) override;

    void drawLabel(juce::Graphics& g, juce::Label& label) override;

    void drawPopupMenuBackground(juce::Graphics& g, int width, int height) override;

    void drawPopupMenuItem(juce::Graphics& g, const juce::Rectangle<int>& area,
                           bool isSeparator, bool isActive, bool isHighlighted,
                           bool isTicked, bool hasSubMenu,
                           const juce::String& text, const juce::String& shortcutKeyText,
                           const juce::Drawable* icon, const juce::Colour* textColour) override;

    juce::Font getLabelFont(juce::Label& label) override;

    // Palette (names kept for existing views; values are the warm hardware palette)
    static const juce::Colour bgDark;
    static const juce::Colour bgPanel;
    static const juce::Colour bgControl;
    static const juce::Colour cyanGlow;   // primary highlight (warm amber)
    static const juce::Colour amberGlow;  // illumination
    static const juce::Colour slateBorder;
    static const juce::Colour slateHighlight;
    static const juce::Colour textLight;  // warm cream
    static const juce::Colour textDim;
    static const juce::Colour soloYellow;
    static const juce::Colour muteRed;
    static const juce::Colour phaseBlue;
    static const juce::Colour titleAmber;
    static const juce::Colour meterGreen;

    static const juce::Colour sslBlue;
    static const juce::Colour sslGreen;
    static const juce::Colour sslYellow;
    static const juce::Colour sslRed;
    static const juce::Colour sslAmber;
    static const juce::Colour sslCyan;
    static const juce::Colour sslWhite;
    static const juce::Colour sslDark;
    static const juce::Colour sslPanel;

    // Channel accent colours (slots 1 to 6) and matching waveform tints
    static juce::Colour getSlotColor(int slotIndex);
    static juce::Colour getSlotWaveColor(int slotIndex);

    static juce::Font boldFont(float height);
};

} // namespace yeahwaah
