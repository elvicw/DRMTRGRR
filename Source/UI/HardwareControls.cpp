#include "HardwareControls.h"
#include "SlateLookAndFeel.h"
#include <cmath>

namespace yeahwaah
{
namespace hw
{

namespace
{
juce::Image createMetalTexture()
{
    juce::Image img(juce::Image::ARGB, kDesignWidth, kDesignHeight, true);
    juce::Graphics g(img);

    juce::ColourGradient base(juce::Colour(0xff35332d), 0.0f, 0.0f, juce::Colour(0xff211f1b), 0.0f, static_cast<float>(kDesignHeight), false);
    base.addColour(0.5, juce::Colour(0xff2b2924));
    g.setGradientFill(base);
    g.fillAll();

    juce::Random rng(20241125);
    for (int y = 0; y < kDesignHeight; ++y)
    {
        const bool light = rng.nextBool();
        g.setColour((light ? juce::Colours::white : juce::Colours::black).withAlpha(rng.nextFloat() * 0.05f));
        g.drawHorizontalLine(y, 0.0f, static_cast<float>(kDesignWidth));
    }

    for (int i = 0; i < 2600; ++i)
    {
        const float x = rng.nextFloat() * kDesignWidth;
        const float y = rng.nextFloat() * kDesignHeight;
        const float len = 60.0f + rng.nextFloat() * 520.0f;
        g.setColour((rng.nextBool() ? juce::Colours::white : juce::Colours::black).withAlpha(0.012f + rng.nextFloat() * 0.04f));
        g.drawHorizontalLine(static_cast<int>(y), x, x + len);
    }

    juce::ColourGradient vignette(juce::Colours::transparentBlack, kDesignWidth * 0.5f, kDesignHeight * 0.5f,
                                  juce::Colours::black.withAlpha(0.55f), 0.0f, 0.0f, true);
    g.setGradientFill(vignette);
    g.fillAll();
    return img;
}
} // namespace

void drawBrushedMetal(juce::Graphics& g, juce::Rectangle<int> area)
{
    static const juce::Image texture = createMetalTexture();
    g.drawImage(texture, area.toFloat(), juce::RectanglePlacement::stretchToFit);
}

void drawScrew(juce::Graphics& g, juce::Point<float> c, float r, float rot)
{
    g.setColour(juce::Colours::black.withAlpha(0.7f));
    g.fillEllipse(c.x - r - 1.0f, c.y - r, (r + 1.0f) * 2.0f, (r + 1.0f) * 2.0f);

    juce::ColourGradient dome(juce::Colour(0xff777166), c.x - r * 0.5f, c.y - r * 0.6f,
                              juce::Colour(0xff1a1915), c.x + r * 0.7f, c.y + r * 0.8f, true);
    g.setGradientFill(dome);
    g.fillEllipse(c.x - r, c.y - r, r * 2.0f, r * 2.0f);
    g.setColour(juce::Colour(0xff0b0a08));
    g.drawEllipse(c.x - r, c.y - r, r * 2.0f, r * 2.0f, 1.2f);

    const float s = r * 0.7f;
    g.setColour(juce::Colour(0xff0b0a08));
    for (int i = 0; i < 2; ++i)
    {
        const float a = rot + static_cast<float>(i) * juce::MathConstants<float>::halfPi;
        g.drawLine(c.x - s * std::cos(a), c.y - s * std::sin(a), c.x + s * std::cos(a), c.y + s * std::sin(a), 1.8f);
    }
}

void drawPanel(juce::Graphics& g, juce::Rectangle<float> a, float corner)
{
    g.setColour(juce::Colours::black.withAlpha(0.32f));
    g.fillRoundedRectangle(a, corner);
    g.setColour(juce::Colours::black.withAlpha(0.85f));
    g.drawRoundedRectangle(a, corner, 2.2f);
    g.setColour(juce::Colour(0xff77705f).withAlpha(0.35f));
    g.drawRoundedRectangle(a.reduced(2.5f), corner - 1.5f, 1.0f);
}

void drawEngravedText(juce::Graphics& g, const juce::String& text, juce::Rectangle<int> area,
                      float height, juce::Colour colour, juce::Justification just)
{
    g.setFont(SlateLookAndFeel::boldFont(height));
    g.setColour(juce::Colours::black.withAlpha(0.8f));
    g.drawText(text, area.translated(1, 2), just, false);
    g.setColour(colour);
    g.drawText(text, area, just, false);
}

void drawLedBar(juce::Graphics& g, juce::Rectangle<float> a, float level, int numLeds)
{
    const float gap = 3.0f;
    const float ledH = (a.getHeight() - gap * static_cast<float>(numLeds - 1)) / static_cast<float>(numLeds);
    const int lit = static_cast<int>(std::ceil(juce::jlimit(0.0f, 1.0f, level) * static_cast<float>(numLeds)));

    for (int i = 0; i < numLeds; ++i)
    {
        const float frac = static_cast<float>(i) / static_cast<float>(numLeds - 1);
        juce::Colour c = frac < 0.6f ? juce::Colour(0xffe8c36a)
                       : frac < 0.85f ? juce::Colour(0xfff0a830)
                                      : juce::Colour(0xffe5532a);
        const float y = a.getBottom() - static_cast<float>(i + 1) * ledH - static_cast<float>(i) * gap;
        juce::Rectangle<float> r(a.getX(), y, a.getWidth(), ledH);

        if (i < lit)
        {
            g.setColour(c.withAlpha(0.30f));
            g.fillRoundedRectangle(r.expanded(1.5f), 2.0f);
            g.setColour(c);
            g.fillRoundedRectangle(r, 1.5f);
        }
        else
        {
            g.setColour(c.withAlpha(0.13f));
            g.fillRoundedRectangle(r, 1.5f);
            g.setColour(juce::Colours::black.withAlpha(0.6f));
            g.drawRoundedRectangle(r, 1.5f, 0.8f);
        }
    }
}

juce::String formatValue(double value, int decimals, const juce::String& suffix)
{
    return juce::String(value, decimals) + suffix;
}

// ---------------------------------------------------------------------------

HardwareKnob::HardwareKnob(juce::AudioProcessorValueTreeState& apvts, const juce::String& paramId,
                           const juce::String& cap, const juce::String& suffix, int decimals,
                           TextFn textFn, float captionH)
    : caption(cap), captionHeight(captionH)
{
    slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setRotaryParameters(juce::MathConstants<float>::pi * 1.25f, juce::MathConstants<float>::pi * 2.75f, true);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 90, 26);
    slider.setScrollWheelEnabled(true);
    addAndMakeVisible(slider);

    attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, paramId, slider);

    if (textFn != nullptr)
        slider.textFromValueFunction = textFn;
    else if (suffix.isNotEmpty() || decimals >= 0)
        slider.textFromValueFunction = [suffix, decimals](double v) { return formatValue(v, decimals, suffix); };

    slider.updateText();
    if (auto* p = apvts.getParameter(paramId))
    {
        slider.setDoubleClickReturnValue(true, p->convertFrom0to1(p->getDefaultValue()));
        slider.setTooltip(p->getName(64) + " (double-click to reset)");
    }
}

void HardwareKnob::paint(juce::Graphics& g)
{
    if (captionHeight > 0.0f)
        drawEngravedText(g, caption, getLocalBounds().removeFromTop(static_cast<int>(captionHeight)), captionHeight * 0.92f,
                         SlateLookAndFeel::textLight);
}

void HardwareKnob::resized()
{
    const int cap = static_cast<int>(captionHeight);
    slider.setBounds(getLocalBounds().withTrimmedTop(cap));
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, juce::jmin(92, getWidth()), 26);
}

// ---------------------------------------------------------------------------

void HardwareButton::paintButton(juce::Graphics& g, bool highlighted, bool down)
{
    auto b = getLocalBounds().toFloat().reduced(1.0f);
    const bool on = getToggleState();
    const auto amber = SlateLookAndFeel::amberGlow;

    if (style == Style::Lamp || style == Style::Led)
    {
        const bool round = style == Style::Led;
        auto drawShape = [&](juce::Rectangle<float> r, juce::Colour c, bool fill, float thick) {
            g.setColour(c);
            if (round) { fill ? g.fillEllipse(r) : g.drawEllipse(r, thick); }
            else       { fill ? g.fillRoundedRectangle(r, 6.0f) : g.drawRoundedRectangle(r, 6.0f, thick); }
        };

        drawShape(b, juce::Colour(0xff0a0907), true, 0.0f);
        drawShape(b, juce::Colour(0xff55503f), false, 1.4f);

        auto lamp = b.reduced(round ? 2.5f : 4.5f);
        if (on)
        {
            drawShape(lamp.expanded(5.0f), amber.withAlpha(0.20f), true, 0.0f);
            drawShape(lamp.expanded(2.5f), amber.withAlpha(0.28f), true, 0.0f);
            juce::ColourGradient grad(juce::Colour(0xffffe08a), lamp.getCentreX(), lamp.getY(),
                                      juce::Colour(0xfff0a114), lamp.getCentreX(), lamp.getBottom(), false);
            g.setGradientFill(grad);
            if (round) g.fillEllipse(lamp); else g.fillRoundedRectangle(lamp, 4.0f);
        }
        else
        {
            drawShape(lamp, highlighted ? juce::Colour(0xff4a3a1c) : juce::Colour(0xff35290f), true, 0.0f);
            drawShape(lamp, juce::Colour(0xff0a0907), false, 1.0f);
        }
        return;
    }

    g.setColour(juce::Colours::black.withAlpha(0.6f));
    g.fillRoundedRectangle(b.translated(0.0f, 2.0f), 6.0f);

    juce::Colour top = highlighted ? juce::Colour(0xff4a463c) : juce::Colour(0xff3b3830);
    juce::Colour bottom(0xff1b1a16);
    if (down) std::swap(top, bottom);
    juce::ColourGradient grad(top, b.getX(), b.getY(), bottom, b.getX(), b.getBottom(), false);
    g.setGradientFill(grad);
    g.fillRoundedRectangle(b, 6.0f);

    if (on)
    {
        g.setColour(amber.withAlpha(0.14f));
        g.fillRoundedRectangle(b, 6.0f);
    }

    g.setColour(on ? amber.withAlpha(0.9f) : juce::Colour(0xff0a0907));
    g.drawRoundedRectangle(b, 6.0f, on ? 1.8f : 1.5f);
    g.setColour(juce::Colour(0xff8a836f).withAlpha(0.30f));
    g.drawRoundedRectangle(b.reduced(2.2f), 4.5f, 1.0f);

    g.setFont(SlateLookAndFeel::boldFont(b.getHeight() * 0.50f));
    g.setColour(juce::Colours::black.withAlpha(0.7f));
    g.drawText(getButtonText(), getLocalBounds().translated(1, 2), juce::Justification::centred, false);
    g.setColour(on ? amber : SlateLookAndFeel::textLight);
    g.drawText(getButtonText(), getLocalBounds(), juce::Justification::centred, false);
}

// ---------------------------------------------------------------------------

ValuePad::ValuePad(juce::AudioProcessorValueTreeState& apvts, const juce::String& paramId,
                   const juce::String& cap, const juce::String& suffix, int decimals)
    : caption(cap)
{
    setSliderStyle(juce::Slider::LinearBar);
    setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    setSliderSnapsToMousePosition(false);

    attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, paramId, *this);
    textFromValueFunction = [suffix, decimals](double v) { return formatValue(v, decimals, suffix); };

    if (auto* p = apvts.getParameter(paramId))
    {
        setDoubleClickReturnValue(true, p->convertFrom0to1(p->getDefaultValue()));
        setTooltip(p->getName(64) + ": drag to adjust, double-click to reset");
    }
}

void ValuePad::paint(juce::Graphics& g)
{
    auto b = getLocalBounds().toFloat().reduced(1.0f);
    const bool active = isMouseOverOrDragging();
    const bool lit = getValue() > getMinimum() + 1.0e-6;
    const auto amber = SlateLookAndFeel::amberGlow;

    g.setColour(juce::Colours::black.withAlpha(0.6f));
    g.fillRoundedRectangle(b.translated(0.0f, 2.0f), 7.0f);
    juce::ColourGradient grad(active ? juce::Colour(0xff45423a) : juce::Colour(0xff35332c), b.getX(), b.getY(),
                              juce::Colour(0xff15140f), b.getX(), b.getBottom(), false);
    g.setGradientFill(grad);
    g.fillRoundedRectangle(b, 7.0f);
    g.setColour(juce::Colour(0xff0a0907));
    g.drawRoundedRectangle(b, 7.0f, 1.6f);
    g.setColour(juce::Colour(0xff8a836f).withAlpha(0.25f));
    g.drawRoundedRectangle(b.reduced(2.4f), 5.5f, 1.0f);

    const float lampS = b.getHeight() * 0.58f;
    juce::Rectangle<float> lamp(b.getX() + 10.0f, b.getCentreY() - lampS * 0.5f, lampS, lampS);
    if (lit)
    {
        g.setColour(amber.withAlpha(0.28f));
        g.fillRoundedRectangle(lamp.expanded(4.0f), 6.0f);
        juce::ColourGradient lg(juce::Colour(0xffffe08a), lamp.getCentreX(), lamp.getY(),
                                juce::Colour(0xfff0a114), lamp.getCentreX(), lamp.getBottom(), false);
        g.setGradientFill(lg);
        g.fillRoundedRectangle(lamp, 4.0f);
    }
    else
    {
        g.setColour(juce::Colour(0xff35290f));
        g.fillRoundedRectangle(lamp, 4.0f);
    }
    g.setColour(juce::Colour(0xff0a0907));
    g.drawRoundedRectangle(lamp, 4.0f, 1.2f);

    const juce::String text = active ? getTextFromValue(getValue()) : caption;
    auto textArea = getLocalBounds().withTrimmedLeft(static_cast<int>(lamp.getRight() + 6.0f - b.getX()));
    g.setFont(SlateLookAndFeel::boldFont(b.getHeight() * 0.36f));
    g.setColour(juce::Colours::black.withAlpha(0.7f));
    g.drawText(text, textArea.translated(1, 1), juce::Justification::centred, false);
    g.setColour(lit ? SlateLookAndFeel::textLight : SlateLookAndFeel::textDim);
    g.drawText(text, textArea, juce::Justification::centred, false);
}

} // namespace hw
} // namespace yeahwaah
