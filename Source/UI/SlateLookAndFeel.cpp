#include "SlateLookAndFeel.h"
#include <cmath>

namespace yeahwaah
{

const juce::Colour SlateLookAndFeel::bgDark      = juce::Colour(0xff121316);
const juce::Colour SlateLookAndFeel::bgPanel     = juce::Colour(0xff1a1d22);
const juce::Colour SlateLookAndFeel::bgControl   = juce::Colour(0xff242830);
const juce::Colour SlateLookAndFeel::cyanGlow    = juce::Colour(0xff00d2ff);
const juce::Colour SlateLookAndFeel::amberGlow   = juce::Colour(0xffff9d00);
const juce::Colour SlateLookAndFeel::slateBorder = juce::Colour(0xff343a46);
const juce::Colour SlateLookAndFeel::textLight   = juce::Colour(0xffe6eaf0);
const juce::Colour SlateLookAndFeel::textDim     = juce::Colour(0xff8a94a6);
const juce::Colour SlateLookAndFeel::soloYellow  = juce::Colour(0xffffd000);
const juce::Colour SlateLookAndFeel::muteRed     = juce::Colour(0xffff3344);

SlateLookAndFeel::SlateLookAndFeel()
{
    setColour(juce::ResizableWindow::backgroundColourId, bgDark);
    setColour(juce::Slider::textBoxTextColourId, textLight);
    setColour(juce::Slider::textBoxBackgroundColourId, bgControl.darker(0.3f));
    setColour(juce::Slider::textBoxOutlineColourId, slateBorder);
    setColour(juce::ComboBox::backgroundColourId, bgControl);
    setColour(juce::ComboBox::textColourId, textLight);
    setColour(juce::ComboBox::outlineColourId, slateBorder);
    setColour(juce::PopupMenu::backgroundColourId, bgPanel);
    setColour(juce::PopupMenu::textColourId, textLight);
    setColour(juce::PopupMenu::highlightedBackgroundColourId, cyanGlow.withAlpha(0.25f));
}

void SlateLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                                        float sliderPosProportional, float rotaryStartAngle,
                                        float rotaryEndAngle, juce::Slider& slider)
{
    const float radius = juce::jmin(width / 2.0f, height / 2.0f) - 4.0f;
    const float centreX = x + width * 0.5f;
    const float centreY = y + height * 0.5f;
    const float rx = centreX - radius;
    const float ry = centreY - radius;
    const float rw = radius * 2.0f;

    const float angle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);

    // Track arc behind knob
    juce::Path backgroundArc;
    backgroundArc.addCentredArc(centreX, centreY, radius - 2.0f, radius - 2.0f, 0.0f,
                                rotaryStartAngle, rotaryEndAngle, true);
    g.setColour(juce::Colour(0xff0e1013));
    g.strokePath(backgroundArc, juce::PathStrokeType(4.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // Active arc track
    juce::Colour activeColor = slider.getName().containsIgnoreCase("detail") || slider.getName().containsIgnoreCase("gate")
                                ? amberGlow : cyanGlow;
    juce::Path activeArc;
    activeArc.addCentredArc(centreX, centreY, radius - 2.0f, radius - 2.0f, 0.0f,
                            rotaryStartAngle, angle, true);
    g.setColour(activeColor.withAlpha(0.85f));
    g.strokePath(activeArc, juce::PathStrokeType(3.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // Outer metal ring shadow & body
    g.setColour(juce::Colour(0xff090a0c));
    g.fillEllipse(rx + 2.0f, ry + 2.0f, rw - 4.0f, rw - 4.0f);

    juce::ColourGradient knobGradient(juce::Colour(0xff2d323b), centreX, ry + 4.0f,
                                      juce::Colour(0xff181a1f), centreX, ry + rw - 4.0f, false);
    g.setGradientFill(knobGradient);
    g.fillEllipse(rx + 3.0f, ry + 3.0f, rw - 6.0f, rw - 6.0f);

    // Subtle brushed bevel
    g.setColour(slateBorder.brighter(0.2f));
    g.drawEllipse(rx + 3.0f, ry + 3.0f, rw - 6.0f, rw - 6.0f, 1.2f);

    // Knurled inner rim
    g.setColour(juce::Colour(0xff14161a));
    g.drawEllipse(rx + 6.0f, ry + 6.0f, rw - 12.0f, rw - 12.0f, 1.5f);

    // Glowing pointer tick
    const float indicatorLen = radius * 0.55f;
    const float pointerThickness = 3.0f;
    juce::Point<float> p1(centreX + std::sin(angle) * (radius * 0.25f),
                          centreY - std::cos(angle) * (radius * 0.25f));
    juce::Point<float> p2(centreX + std::sin(angle) * indicatorLen,
                          centreY - std::cos(angle) * indicatorLen);

    // Pointer glow
    g.setColour(activeColor.withAlpha(0.4f));
    g.drawLine(p1.x, p1.y, p2.x, p2.y, pointerThickness + 3.0f);

    // Pointer core
    g.setColour(activeColor.brighter(0.3f));
    g.drawLine(p1.x, p1.y, p2.x, p2.y, pointerThickness);
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
        // Background track groove
        g.setColour(juce::Colour(0xff0d0f12));
        g.fillRoundedRectangle(trackX, (float)y, trackWidth, (float)height, 2.5f);
        g.setColour(slateBorder);
        g.drawRoundedRectangle(trackX, (float)y, trackWidth, (float)height, 2.5f, 1.0f);

        // Active fill from bottom to sliderPos
        float fillY = sliderPos;
        float fillH = (y + height) - sliderPos;
        g.setColour(cyanGlow.withAlpha(0.6f));
        g.fillRoundedRectangle(trackX + 1.0f, fillY, trackWidth - 2.0f, fillH, 1.5f);

        // Fader handle (Slate metal cap)
        float handleW = (float)width - 6.0f;
        float handleH = 14.0f;
        float handleX = x + 3.0f;
        float handleY = sliderPos - handleH * 0.5f;

        g.setColour(juce::Colour(0xff090a0d));
        g.fillRoundedRectangle(handleX, handleY + 1.0f, handleW, handleH, 2.0f);

        juce::ColourGradient capGrad(juce::Colour(0xff3c434f), handleX, handleY,
                                     juce::Colour(0xff1e2127), handleX, handleY + handleH, false);
        g.setGradientFill(capGrad);
        g.fillRoundedRectangle(handleX, handleY, handleW, handleH, 2.0f);

        g.setColour(slateBorder.brighter(0.2f));
        g.drawRoundedRectangle(handleX, handleY, handleW, handleH, 2.0f, 1.0f);

        // Center glowing tick on handle
        g.setColour(cyanGlow);
        g.fillRect(handleX + 2.0f, handleY + handleH * 0.5f - 1.0f, handleW - 4.0f, 2.0f);
    }
    else
    {
        // Horizontal slider
        float trackY = y + height * 0.5f - trackWidth * 0.5f;
        g.setColour(juce::Colour(0xff0d0f12));
        g.fillRoundedRectangle((float)x, trackY, (float)width, trackWidth, 2.5f);
        g.setColour(slateBorder);
        g.drawRoundedRectangle((float)x, trackY, (float)width, trackWidth, 2.5f, 1.0f);

        // Active fill
        float fillW = sliderPos - x;
        g.setColour(cyanGlow.withAlpha(0.6f));
        g.fillRoundedRectangle((float)x + 1.0f, trackY + 1.0f, fillW, trackWidth - 2.0f, 1.5f);

        // Handle
        float handleW = 14.0f;
        float handleH = (float)height - 6.0f;
        float handleX = sliderPos - handleW * 0.5f;
        float handleY = y + 3.0f;

        g.setColour(juce::Colour(0xff2d323c));
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
    else if (name.containsIgnoreCase("test"))
        activeColor = juce::Colour(0xff22dd88);

    if (isToggled)
    {
        // Lit / Active State
        g.setColour(activeColor.withAlpha(0.25f));
        g.fillRoundedRectangle(bounds, 3.0f);
        g.setColour(activeColor);
        g.drawRoundedRectangle(bounds, 3.0f, 1.5f);
    }
    else
    {
        // Dark metallic button
        juce::Colour btnBg = shouldDrawButtonAsDown ? bgControl.darker(0.3f)
                           : shouldDrawButtonAsHighlighted ? bgControl.brighter(0.15f)
                           : bgControl;
        g.setColour(btnBg);
        g.fillRoundedRectangle(bounds, 3.0f);
        g.setColour(slateBorder);
        g.drawRoundedRectangle(bounds, 3.0f, 1.0f);
    }

    // Jewel LED indicator in corner if button is toggleable
    if (button.isToggleable())
    {
        float ledSize = 4.5f;
        float ledX = bounds.getX() + 4.0f;
        float ledY = bounds.getY() + 4.0f;
        if (isToggled)
        {
            g.setColour(activeColor.withAlpha(0.4f));
            g.fillEllipse(ledX - 1.5f, ledY - 1.5f, ledSize + 3.0f, ledSize + 3.0f);
            g.setColour(activeColor.brighter(0.4f));
            g.fillEllipse(ledX, ledY, ledSize, ledSize);
        }
        else
        {
            g.setColour(juce::Colour(0xff181a1e));
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

    g.setColour(bgControl);
    g.fillRoundedRectangle(bounds, 3.0f);
    g.setColour(slateBorder);
    g.drawRoundedRectangle(bounds, 3.0f, 1.0f);

    // Arrow icon
    juce::Path arrow;
    float arrowX = width - 16.0f;
    float arrowY = height * 0.5f - 2.0f;
    arrow.startNewSubPath(arrowX, arrowY);
    arrow.lineTo(arrowX + 4.0f, arrowY + 4.0f);
    arrow.lineTo(arrowX + 8.0f, arrowY);

    g.setColour(cyanGlow);
    g.strokePath(arrow, juce::PathStrokeType(1.8f));
}

void SlateLookAndFeel::drawPopupMenuBackground(juce::Graphics& g, int width, int height)
{
    g.setColour(bgPanel);
    g.fillRect(0, 0, width, height);
    g.setColour(slateBorder.brighter(0.3f));
    g.drawRect(0, 0, width, height, 1);
}

} // namespace yeahwaah
