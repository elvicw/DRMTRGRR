#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>

namespace yeahwaah
{
namespace hw
{

// Reference design space; the whole editor is laid out in these units and scaled to the window.
constexpr int kDesignWidth = 1536;
constexpr int kDesignHeight = 1024;

// Chassis drawing helpers
void drawBrushedMetal(juce::Graphics& g, juce::Rectangle<int> area);
void drawScrew(juce::Graphics& g, juce::Point<float> centre, float radius, float rotationRadians = 0.4f);
void drawPanel(juce::Graphics& g, juce::Rectangle<float> area, float corner = 9.0f);
void drawEngravedText(juce::Graphics& g, const juce::String& text, juce::Rectangle<int> area,
                      float height, juce::Colour colour, juce::Justification just = juce::Justification::centred);
void drawLedBar(juce::Graphics& g, juce::Rectangle<float> area, float level, int numLeds);

using TextFn = std::function<juce::String(double)>;
juce::String formatValue(double value, int decimals, const juce::String& suffix);

// Rotary knob with caption above and recessed value readout below, bound to an APVTS parameter.
class HardwareKnob : public juce::Component
{
public:
    HardwareKnob(juce::AudioProcessorValueTreeState& apvts, const juce::String& paramId,
                 const juce::String& caption, const juce::String& suffix = {}, int decimals = 1,
                 TextFn textFn = nullptr, float captionHeight = 18.0f);

    void paint(juce::Graphics& g) override;
    void resized() override;

    juce::Slider& getSlider() { return slider; }

private:
    juce::Slider slider;
    juce::String caption;
    float captionHeight;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
};

// Illuminated button: a lamp square, a round LED, or a labelled key (M / S / phase).
class HardwareButton : public juce::Button
{
public:
    enum class Style { Lamp, Led, Key };

    HardwareButton(const juce::String& text, Style s) : juce::Button(text), style(s)
    {
        setClickingTogglesState(true);
    }

    void paintButton(juce::Graphics& g, bool highlighted, bool down) override;

private:
    Style style;
};

// Horizontal drag pad (lit lamp + caption); shows the value while hovered or dragged.
class ValuePad : public juce::Slider
{
public:
    ValuePad(juce::AudioProcessorValueTreeState& apvts, const juce::String& paramId,
             const juce::String& caption, const juce::String& suffix, int decimals);

    void paint(juce::Graphics& g) override;
    void mouseEnter(const juce::MouseEvent& e) override { juce::Slider::mouseEnter(e); repaint(); }
    void mouseExit(const juce::MouseEvent& e) override { juce::Slider::mouseExit(e); repaint(); }

private:
    juce::String caption;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
};

} // namespace hw
} // namespace yeahwaah
