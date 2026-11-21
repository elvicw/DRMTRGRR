#include "SlateLookAndFeel.h"
#include <cmath>

namespace yeahwaah
{

const juce::Colour SlateLookAndFeel::bgDark      = juce::Colour(0xff0f1115);
const juce::Colour SlateLookAndFeel::bgPanel     = juce::Colour(0xff181b22);
const juce::Colour SlateLookAndFeel::bgControl   = juce::Colour(0xff222630);
const juce::Colour SlateLookAndFeel::cyanGlow    = juce::Colour(0xff00e5ff);
const juce::Colour SlateLookAndFeel::amberGlow   = juce::Colour(0xffffa600);
const juce::Colour SlateLookAndFeel::slateBorder = juce::Colour(0xff333a48);
const juce::Colour SlateLookAndFeel::textLight   = juce::Colour(0xffedf1f7);
const juce::Colour SlateLookAndFeel::textDim     = juce::Colour(0xff8b95a5);
const juce::Colour SlateLookAndFeel::soloYellow  = juce::Colour(0xffffd700);
const juce::Colour SlateLookAndFeel::muteRed     = juce::Colour(0xffff2d55);

SlateLookAndFeel::SlateLookAndFeel()
{
    setColour(juce::ResizableWindow::backgroundColourId, bgDark);
    setColour(juce::Slider::textBoxTextColourId, textLight);
    setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour(0xff14171d));
    setColour(juce::Slider::textBoxOutlineColourId, slateBorder);
    setColour(juce::ComboBox::backgroundColourId, bgControl);
    setColour(juce::ComboBox::textColourId, textLight);
    setColour(juce::ComboBox::outlineColourId, slateBorder);
    setColour(juce::PopupMenu::backgroundColourId, bgPanel);
    setColour(juce::PopupMenu::textColourId, textLight);
    setColour(juce::PopupMenu::highlightedBackgroundColourId, cyanGlow.withAlpha(0.28f));
}

void SlateLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                                        float sliderPosProportional, float rotaryStartAngle,
                                        float rotaryEndAngle, juce::Slider& slider)
{
    const float radius = juce::jmin(width / 2.0f, height / 2.0f) - 3.5f;
    const float centreX = x + width * 0.5f;
    const float centreY = y + height * 0.5f;
    const float rx = centreX - radius;
    const float ry = centreY - radius;
    const float rw = radius * 2.0f;

    const float angle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);

    // 1. Outer Dark Bezel / Socket with drop shadow
    g.setColour(juce::Colour(0xff08090c));
    g.fillEllipse(rx - 1.0f, ry - 1.0f, rw + 2.0f, rw + 2.0f);

    // 2. Background Track Arc (recessed groove)
    juce::Path backgroundArc;
    backgroundArc.addCentredArc(centreX, centreY, radius - 1.5f, radius - 1.5f, 0.0f,
                                rotaryStartAngle, rotaryEndAngle, true);
    g.setColour(juce::Colour(0xff0a0c0f));
    g.strokePath(backgroundArc, juce::PathStrokeType(4.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    g.setColour(slateBorder.darker(0.4f));
    g.strokePath(backgroundArc, juce::PathStrokeType(1.0f));

    // 3. Glowing Active Arc Track
    juce::Colour activeColor = slider.getName().containsIgnoreCase("detail") || slider.getName().containsIgnoreCase("gate")
                                ? amberGlow : cyanGlow;
    if (sliderPosProportional > 0.005f)
    {
        juce::Path activeArc;
        activeArc.addCentredArc(centreX, centreY, radius - 1.5f, radius - 1.5f, 0.0f,
                                rotaryStartAngle, angle, true);
        // Outer halo glow
        g.setColour(activeColor.withAlpha(0.35f * (0.4f + 0.6f * sliderPosProportional)));
        g.strokePath(activeArc, juce::PathStrokeType(6.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        // Bright core arc
        g.setColour(activeColor);
        g.strokePath(activeArc, juce::PathStrokeType(3.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }

    // 4. Machined Aluminum Knob Body
    const float knobRadius = radius - 5.0f;
    const float kx = centreX - knobRadius;
    const float ky = centreY - knobRadius;
    const float kw = knobRadius * 2.0f;

    // Outer knurled notch rim
    g.setColour(juce::Colour(0xff121419));
    g.fillEllipse(kx - 1.5f, ky - 1.5f, kw + 3.0f, kw + 3.0f);

    // Radial brushed metallic gradient on knob top
    juce::ColourGradient knobGrad(juce::Colour(0xff3a414e), centreX - knobRadius * 0.4f, centreY - knobRadius * 0.4f,
                                  juce::Colour(0xff191c22), centreX + knobRadius * 0.6f, centreY + knobRadius * 0.6f, true);
    g.setGradientFill(knobGrad);
    g.fillEllipse(kx, ky, kw, kw);

    // Polished metallic bevel highlight
    g.setColour(juce::Colour(0xff556075));
    g.drawEllipse(kx, ky, kw, kw, 1.2f);

    // Concentric machined circular grooves
    g.setColour(juce::Colour(0xff14171d).withAlpha(0.7f));
    g.drawEllipse(kx + 4.0f, ky + 4.0f, kw - 8.0f, kw - 8.0f, 1.0f);
    g.drawEllipse(kx + 8.0f, ky + 8.0f, kw - 16.0f, kw - 16.0f, 0.8f);

    // 5. High-Intensity Glowing Pointer Tick
    const float indicatorStart = knobRadius * 0.25f;
    const float indicatorLen = knobRadius - 1.5f;
    juce::Point<float> p1(centreX + std::sin(angle) * indicatorStart,
                          centreY - std::cos(angle) * indicatorStart);
    juce::Point<float> p2(centreX + std::sin(angle) * indicatorLen,
                          centreY - std::cos(angle) * indicatorLen);

    // Pointer bloom glow
    g.setColour(activeColor.withAlpha(0.6f));
    g.drawLine(p1.x, p1.y, p2.x, p2.y, 5.0f);

    // Pointer core
    g.setColour(juce::Colours::white);
    g.drawLine(p1.x, p1.y, p2.x, p2.y, 2.4f);
}

void SlateLookAndFeel::drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
                                        float sliderPos, float /*minSliderPos*/, float /*maxSliderPos*/,
                                        const juce::Slider::SliderStyle /*style*/, juce::Slider& slider)
{
    const bool isVertical = slider.isVertical();
    const float trackWidth = 5.0f;

    if (isVertical)
    {
        float trackX = x + width * 0.5f - trackWidth * 0.5f;

        // Fader slot groove with metallic depth
        g.setColour(juce::Colour(0xff090b0e));
        g.fillRoundedRectangle(trackX, (float)y, trackWidth, (float)height, 2.5f);
        g.setColour(slateBorder.darker(0.3f));
        g.drawRoundedRectangle(trackX, (float)y, trackWidth, (float)height, 2.5f, 1.0f);

        // Calibrated tick marks on left and right
        g.setColour(slateBorder.withAlpha(0.6f));
        for (int ty = y + 8; ty <= y + height - 8; ty += 14)
        {
            g.drawHorizontalLine(ty, trackX - 4.0f, trackX - 1.0f);
            g.drawHorizontalLine(ty, trackX + trackWidth + 1.0f, trackX + trackWidth + 4.0f);
        }

        // Active fill bar
        float fillY = sliderPos;
        float fillH = (y + height) - sliderPos;
        if (fillH > 2.0f)
        {
            g.setColour(cyanGlow.withAlpha(0.65f));
            g.fillRoundedRectangle(trackX + 1.0f, fillY, trackWidth - 2.0f, fillH, 1.5f);
        }

        // Slate Machined Metal Fader Handle Cap
        float handleW = (float)width - 4.0f;
        float handleH = 16.0f;
        float handleX = x + 2.0f;
        float handleY = sliderPos - handleH * 0.5f;

        // Drop shadow
        g.setColour(juce::Colour(0xff06070a));
        g.fillRoundedRectangle(handleX, handleY + 2.0f, handleW, handleH, 2.5f);

        // Brushed metal gradient
        juce::ColourGradient capGrad(juce::Colour(0xff4a5363), handleX, handleY,
                                     juce::Colour(0xff1d2128), handleX, handleY + handleH, false);
        g.setGradientFill(capGrad);
        g.fillRoundedRectangle(handleX, handleY, handleW, handleH, 2.5f);

        g.setColour(slateBorder.brighter(0.4f));
        g.drawRoundedRectangle(handleX, handleY, handleW, handleH, 2.5f, 1.0f);

        // Glowing center indicator line on fader
        g.setColour(cyanGlow.withAlpha(0.5f));
        g.fillRect(handleX + 3.0f, handleY + handleH * 0.5f - 1.5f, handleW - 6.0f, 3.0f);
        g.setColour(juce::Colours::white);
        g.fillRect(handleX + 4.0f, handleY + handleH * 0.5f - 0.5f, handleW - 8.0f, 1.0f);
    }
    else
    {
        // Horizontal mini slider
        float trackY = y + height * 0.5f - trackWidth * 0.5f;
        g.setColour(juce::Colour(0xff090b0e));
        g.fillRoundedRectangle((float)x, trackY, (float)width, trackWidth, 2.5f);
        g.setColour(slateBorder);
        g.drawRoundedRectangle((float)x, trackY, (float)width, trackWidth, 2.5f, 1.0f);

        float fillW = std::max(0.0f, sliderPos - x);
        g.setColour(cyanGlow.withAlpha(0.7f));
        g.fillRoundedRectangle((float)x + 1.0f, trackY + 1.0f, fillW, trackWidth - 2.0f, 1.5f);

        float handleW = 12.0f;
        float handleH = (float)height - 4.0f;
        float handleX = sliderPos - handleW * 0.5f;
        float handleY = y + 2.0f;

        g.setColour(juce::Colour(0xff333a46));
        g.fillRoundedRectangle(handleX, handleY, handleW, handleH, 2.0f);
        g.setColour(cyanGlow);
        g.fillRect(handleX + handleW * 0.5f - 1.0f, handleY + 2.0f, 2.0f, handleH - 4.0f);
    }
}

void SlateLookAndFeel::drawButtonBackground(juce::Graphics& g, juce::Button& button,
                                            const juce::Colour& /*backgroundColour*/,
                                            bool shouldDrawButtonAsHighlighted,
                                            bool shouldDrawButtonAsDown)
{
    auto bounds = button.getLocalBounds().toFloat().reduced(1.0f);
    bool isToggled = button.getToggleState();
    auto name = button.getButtonText();

    juce::Colour activeColor = cyanGlow;
    if (name.containsIgnoreCase("listen") || name.containsIgnoreCase("gate"))
        activeColor = amberGlow;
    else if (name.equalsIgnoreCase("S"))
        activeColor = soloYellow;
    else if (name.equalsIgnoreCase("M"))
        activeColor = muteRed;
    else if (name.containsIgnoreCase("hit") || name.containsIgnoreCase("test"))
        activeColor = juce::Colour(0xff00ff88);

    if (isToggled || shouldDrawButtonAsDown)
    {
        // Glowing Active / Depressed State
        g.setColour(activeColor.withAlpha(0.28f));
        g.fillRoundedRectangle(bounds, 3.5f);
        g.setColour(activeColor);
        g.drawRoundedRectangle(bounds, 3.5f, 1.5f);

        // Inner jewel glow
        g.setColour(activeColor.withAlpha(0.15f));
        g.fillRoundedRectangle(bounds.reduced(2.0f), 2.5f);
    }
    else
    {
        // Dark Tactile Brushed Hardware Button
        juce::Colour btnTop = shouldDrawButtonAsHighlighted ? juce::Colour(0xff3a414e) : juce::Colour(0xff292e38);
        juce::Colour btnBottom = shouldDrawButtonAsHighlighted ? juce::Colour(0xff252a33) : juce::Colour(0xff1a1d24);

        juce::ColourGradient btnGrad(btnTop, bounds.getX(), bounds.getY(),
                                     btnBottom, bounds.getX(), bounds.getBottom(), false);
        g.setGradientFill(btnGrad);
        g.fillRoundedRectangle(bounds, 3.5f);

        g.setColour(slateBorder.brighter(0.15f));
        g.drawRoundedRectangle(bounds, 3.5f, 1.0f);
    }

    // Jewel LED indicator
    if (button.isToggleable())
    {
        float ledSize = 4.5f;
        float ledX = bounds.getX() + 4.5f;
        float ledY = bounds.getY() + 4.5f;
        if (isToggled)
        {
            g.setColour(activeColor.withAlpha(0.45f));
            g.fillEllipse(ledX - 2.0f, ledY - 2.0f, ledSize + 4.0f, ledSize + 4.0f);
            g.setColour(activeColor.brighter(0.5f));
            g.fillEllipse(ledX, ledY, ledSize, ledSize);
        }
        else
        {
            g.setColour(juce::Colour(0xff121418));
            g.fillEllipse(ledX, ledY, ledSize, ledSize);
            g.setColour(slateBorder);
            g.drawEllipse(ledX, ledY, ledSize, ledSize, 0.8f);
        }
    }
}

void SlateLookAndFeel::drawComboBox(juce::Graphics& g, int width, int height, bool /*isButtonDown*/,
                                    int /*buttonX*/, int /*buttonY*/, int /*buttonW*/, int /*buttonH*/,
                                    juce::ComboBox& box)
{
    auto bounds = box.getLocalBounds().toFloat().reduced(1.0f);

    juce::ColourGradient cbGrad(juce::Colour(0xff282d37), 0.0f, 0.0f,
                                juce::Colour(0xff1a1d24), 0.0f, (float)height, false);
    g.setGradientFill(cbGrad);
    g.fillRoundedRectangle(bounds, 3.5f);

    g.setColour(slateBorder);
    g.drawRoundedRectangle(bounds, 3.5f, 1.0f);

    // Glowing Down Arrow
    juce::Path arrow;
    float arrowX = width - 18.0f;
    float arrowY = height * 0.5f - 2.5f;
    arrow.startNewSubPath(arrowX, arrowY);
    arrow.lineTo(arrowX + 4.5f, arrowY + 5.0f);
    arrow.lineTo(arrowX + 9.0f, arrowY);

    g.setColour(cyanGlow);
    g.strokePath(arrow, juce::PathStrokeType(2.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
}

void SlateLookAndFeel::drawPopupMenuBackground(juce::Graphics& g, int width, int height)
{
    g.setColour(bgPanel);
    g.fillRect(0, 0, width, height);
    g.setColour(slateBorder.brighter(0.3f));
    g.drawRect(0, 0, width, height, 1);
}

} // namespace yeahwaah
