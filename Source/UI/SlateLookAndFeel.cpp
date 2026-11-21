#include "SlateLookAndFeel.h"
#include <cmath>

namespace yeahwaah
{

const juce::Colour SlateLookAndFeel::bgDark         = juce::Colour(0xff0d0f14);
const juce::Colour SlateLookAndFeel::bgPanel        = juce::Colour(0xff161920);
const juce::Colour SlateLookAndFeel::bgControl      = juce::Colour(0xff1e232c);
const juce::Colour SlateLookAndFeel::cyanGlow       = juce::Colour(0xff00e5ff);
const juce::Colour SlateLookAndFeel::amberGlow      = juce::Colour(0xffff9d00);
const juce::Colour SlateLookAndFeel::slateBorder    = juce::Colour(0xff2c3340);
const juce::Colour SlateLookAndFeel::slateHighlight = juce::Colour(0xff48546a);
const juce::Colour SlateLookAndFeel::textLight      = juce::Colour(0xffeff3fa);
const juce::Colour SlateLookAndFeel::textDim        = juce::Colour(0xff828f9f);
const juce::Colour SlateLookAndFeel::soloYellow     = juce::Colour(0xffffcc00);
const juce::Colour SlateLookAndFeel::muteRed        = juce::Colour(0xffff2a55);
const juce::Colour SlateLookAndFeel::phaseBlue      = juce::Colour(0xff00b0ff);

juce::Colour SlateLookAndFeel::getSlotColor(int slotIndex)
{
    static const juce::Colour colors[] = {
        juce::Colour(0xff00e5ff), // 1: Cyan
        juce::Colour(0xffff9e00), // 2: Amber
        juce::Colour(0xffb84dff), // 3: Violet
        juce::Colour(0xff00e676), // 4: Emerald
        juce::Colour(0xffff2d87), // 5: Magenta
        juce::Colour(0xff00b0ff)  // 6: Azure
    };
    int idx = juce::jlimit(0, 5, slotIndex);
    return colors[idx];
}

SlateLookAndFeel::SlateLookAndFeel()
{
    setColour(juce::ResizableWindow::backgroundColourId, bgDark);
    setColour(juce::Slider::textBoxTextColourId, textLight);
    setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour(0xff10131a));
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
    // Maximize rotary knob diameter within the available area
    const float radius = juce::jmin(static_cast<float>(width) * 0.5f, static_cast<float>(height) * 0.5f) - 2.5f;
    const float centreX = static_cast<float>(x) + static_cast<float>(width) * 0.5f;
    const float centreY = static_cast<float>(y) + static_cast<float>(height) * 0.5f;
    const float rx = centreX - radius;
    const float ry = centreY - radius;
    const float rw = radius * 2.0f;

    const float angle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);

    // Color theme based on parameter role
    auto sName = slider.getName();
    juce::Colour activeColor = cyanGlow;
    if (sName.containsIgnoreCase("detail") || sName.containsIgnoreCase("gate") || sName.containsIgnoreCase("hold"))
        activeColor = amberGlow;
    else if (sName.containsIgnoreCase("cut") || sName.containsIgnoreCase("filter"))
        activeColor = juce::Colour(0xff00ffaa);

    // 1. Recessed Outer Bezel Socket with Drop Shadow
    g.setColour(juce::Colour(0xff06070a));
    g.fillEllipse(rx - 2.0f, ry - 2.0f, rw + 4.0f, rw + 4.0f);
    g.setColour(slateBorder.darker(0.3f));
    g.drawEllipse(rx - 1.5f, ry - 1.5f, rw + 3.0f, rw + 3.0f, 1.0f);

    // 2. Peripheral Calibration Tick Dots around the knob
    const int numTicks = 11;
    const float tickRadius = radius + 1.2f;
    for (int i = 0; i < numTicks; ++i)
    {
        float tickFrac = static_cast<float>(i) / static_cast<float>(numTicks - 1);
        float tickAngle = rotaryStartAngle + tickFrac * (rotaryEndAngle - rotaryStartAngle);
        float tx = centreX + std::sin(tickAngle) * tickRadius;
        float ty = centreY - std::cos(tickAngle) * tickRadius;

        bool isPassed = (tickFrac <= sliderPosProportional + 0.02f);
        if (isPassed && sliderPosProportional > 0.01f)
        {
            g.setColour(activeColor.withAlpha(0.85f));
            g.fillEllipse(tx - 1.3f, ty - 1.3f, 2.6f, 2.6f);
        }
        else
        {
            g.setColour(slateBorder.brighter(0.1f));
            g.fillEllipse(tx - 1.0f, ty - 1.0f, 2.0f, 2.0f);
        }
    }

    // 3. Recessed Background Arc Track
    juce::Path backgroundArc;
    backgroundArc.addCentredArc(centreX, centreY, radius - 2.0f, radius - 2.0f, 0.0f,
                                rotaryStartAngle, rotaryEndAngle, true);
    g.setColour(juce::Colour(0xff08090d));
    g.strokePath(backgroundArc, juce::PathStrokeType(5.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    g.setColour(slateBorder.darker(0.5f));
    g.strokePath(backgroundArc, juce::PathStrokeType(1.0f));

    // 4. Glowing Active Arc Track
    if (sliderPosProportional > 0.005f)
    {
        juce::Path activeArc;
        activeArc.addCentredArc(centreX, centreY, radius - 2.0f, radius - 2.0f, 0.0f,
                                rotaryStartAngle, angle, true);
        // Outer halo glow
        g.setColour(activeColor.withAlpha(0.42f));
        g.strokePath(activeArc, juce::PathStrokeType(7.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        // Vivid core arc
        g.setColour(activeColor);
        g.strokePath(activeArc, juce::PathStrokeType(4.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }

    // 5. Machined Aluminum Knob Body
    const float knobRadius = radius - 4.5f;
    const float kx = centreX - knobRadius;
    const float ky = centreY - knobRadius;
    const float kw = knobRadius * 2.0f;

    // Knurled grip notch rim
    g.setColour(juce::Colour(0xff0f1217));
    g.fillEllipse(kx - 1.5f, ky - 1.5f, kw + 3.0f, kw + 3.0f);

    // Textured knurl teeth ticks around the perimeter
    const int knurlSteps = 24;
    for (int k = 0; k < knurlSteps; ++k)
    {
        float a = static_cast<float>(k) * (juce::MathConstants<float>::twoPi / static_cast<float>(knurlSteps));
        float p1x = centreX + std::sin(a) * (knobRadius + 0.5f);
        float p1y = centreY - std::cos(a) * (knobRadius + 0.5f);
        float p2x = centreX + std::sin(a) * (knobRadius + 2.0f);
        float p2y = centreY - std::cos(a) * (knobRadius + 2.0f);
        g.setColour(juce::Colour(0xff2d3442));
        g.drawLine(p1x, p1y, p2x, p2y, 1.0f);
    }

    // Brushed metallic top plate with anisotropic specular highlight
    juce::ColourGradient knobGrad(juce::Colour(0xff4c576a), centreX - knobRadius * 0.45f, centreY - knobRadius * 0.45f,
                                  juce::Colour(0xff161a20), centreX + knobRadius * 0.65f, centreY + knobRadius * 0.65f, true);
    g.setGradientFill(knobGrad);
    g.fillEllipse(kx, ky, kw, kw);

    // Polished aluminum chamfer bevel highlight
    g.setColour(juce::Colour(0xff74839e));
    g.drawEllipse(kx, ky, kw, kw, 1.4f);

    // Concentric machined lathe grooves
    g.setColour(juce::Colour(0xff12151b).withAlpha(0.6f));
    g.drawEllipse(kx + 4.0f, ky + 4.0f, kw - 8.0f, kw - 8.0f, 0.9f);
    g.drawEllipse(kx + 8.0f, ky + 8.0f, kw - 16.0f, kw - 16.0f, 0.7f);

    // Center jewel cap
    g.setColour(juce::Colour(0xff222732));
    g.fillEllipse(centreX - 4.0f, centreY - 4.0f, 8.0f, 8.0f);
    g.setColour(juce::Colour(0xff556278));
    g.drawEllipse(centreX - 4.0f, centreY - 4.0f, 8.0f, 8.0f, 0.9f);

    // 6. High-Intensity Illuminated Pointer Needle
    const float indicatorStart = knobRadius * 0.25f;
    const float indicatorLen = knobRadius - 1.0f;
    juce::Point<float> p1(centreX + std::sin(angle) * indicatorStart,
                          centreY - std::cos(angle) * indicatorStart);
    juce::Point<float> p2(centreX + std::sin(angle) * indicatorLen,
                          centreY - std::cos(angle) * indicatorLen);

    // Pointer bloom halo
    g.setColour(activeColor.withAlpha(0.70f));
    g.drawLine(p1.x, p1.y, p2.x, p2.y, 6.0f);

    // Pointer core line
    g.setColour(juce::Colours::white);
    g.drawLine(p1.x, p1.y, p2.x, p2.y, 2.8f);
}

void SlateLookAndFeel::drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
                                        float sliderPos, float /*minSliderPos*/, float /*maxSliderPos*/,
                                        const juce::Slider::SliderStyle /*style*/, juce::Slider& slider)
{
    const bool isVertical = slider.isVertical();
    const float trackWidth = isVertical ? 8.0f : 5.0f;

    if (isVertical)
    {
        float trackX = static_cast<float>(x) + static_cast<float>(width) * 0.5f - trackWidth * 0.5f;

        // Fader slot groove with metallic depth
        g.setColour(juce::Colour(0xff06070a));
        g.fillRoundedRectangle(trackX, static_cast<float>(y), trackWidth, static_cast<float>(height), 3.5f);
        g.setColour(slateBorder.darker(0.35f));
        g.drawRoundedRectangle(trackX, static_cast<float>(y), trackWidth, static_cast<float>(height), 3.5f, 1.2f);

        // Center guide slit
        g.setColour(juce::Colour(0xff020204));
        g.drawVerticalLine(static_cast<int>(trackX + trackWidth * 0.5f), static_cast<float>(y) + 2.0f, static_cast<float>(y + height) - 2.0f);

        // Calibrated dB tick marks on left and right
        g.setColour(slateBorder.brighter(0.2f));
        for (int ty = y + 10; ty <= y + height - 10; ty += 14)
        {
            g.drawHorizontalLine(ty, trackX - 6.0f, trackX - 2.0f);
            g.drawHorizontalLine(ty, trackX + trackWidth + 2.0f, trackX + trackWidth + 6.0f);
        }

        // Active fill bar
        float fillY = sliderPos;
        float fillH = static_cast<float>(y + height) - sliderPos;
        if (fillH > 2.0f)
        {
            g.setColour(cyanGlow.withAlpha(0.65f));
            g.fillRoundedRectangle(trackX + 1.5f, fillY, trackWidth - 3.0f, fillH, 2.0f);
        }

        // Slate Machined Metal Console Fader Cap (Chunky & Tactile)
        float handleW = static_cast<float>(width) - 4.0f;
        float handleH = 22.0f;
        float handleX = static_cast<float>(x) + 2.0f;
        float handleY = sliderPos - handleH * 0.5f;

        // Drop shadow
        g.setColour(juce::Colour(0xff040507).withAlpha(0.9f));
        g.fillRoundedRectangle(handleX, handleY + 3.0f, handleW, handleH, 3.0f);

        // Brushed metal gradient
        juce::ColourGradient capGrad(juce::Colour(0xff556073), handleX, handleY,
                                     juce::Colour(0xff1c2027), handleX, handleY + handleH, false);
        g.setGradientFill(capGrad);
        g.fillRoundedRectangle(handleX, handleY, handleW, handleH, 3.0f);

        // Chamfer highlights
        g.setColour(juce::Colour(0xff7d8ba3));
        g.drawRoundedRectangle(handleX, handleY, handleW, handleH, 3.0f, 1.2f);

        // Tactile finger grip ridges (3 ridges)
        g.setColour(juce::Colour(0xff12151b));
        g.drawHorizontalLine(static_cast<int>(handleY + 5.0f), handleX + 4.0f, handleX + handleW - 4.0f);
        g.drawHorizontalLine(static_cast<int>(handleY + handleH * 0.5f), handleX + 4.0f, handleX + handleW - 4.0f);
        g.drawHorizontalLine(static_cast<int>(handleY + handleH - 5.0f), handleX + 4.0f, handleX + handleW - 4.0f);

        // Glowing center indicator stripe
        g.setColour(cyanGlow.withAlpha(0.75f));
        g.fillRect(handleX + 3.0f, handleY + handleH * 0.5f - 1.5f, handleW - 6.0f, 3.0f);
        g.setColour(juce::Colours::white);
        g.fillRect(handleX + 4.0f, handleY + handleH * 0.5f - 0.5f, handleW - 8.0f, 1.0f);
    }
    else
    {
        // Horizontal mini slider (Attack, Decay, etc.)
        float trackY = static_cast<float>(y) + static_cast<float>(height) * 0.5f - trackWidth * 0.5f;
        g.setColour(juce::Colour(0xff08090d));
        g.fillRoundedRectangle(static_cast<float>(x), trackY, static_cast<float>(width), trackWidth, 2.5f);
        g.setColour(slateBorder);
        g.drawRoundedRectangle(static_cast<float>(x), trackY, static_cast<float>(width), trackWidth, 2.5f, 1.0f);

        float fillW = std::max(0.0f, sliderPos - static_cast<float>(x));
        g.setColour(cyanGlow.withAlpha(0.7f));
        g.fillRoundedRectangle(static_cast<float>(x) + 1.0f, trackY + 1.0f, fillW, trackWidth - 2.0f, 1.5f);

        // Tactile slider thumb
        float handleW = 14.0f;
        float handleH = static_cast<float>(height) - 2.0f;
        float handleX = sliderPos - handleW * 0.5f;
        float handleY = static_cast<float>(y) + 1.0f;

        g.setColour(juce::Colour(0xff3a4250));
        g.fillRoundedRectangle(handleX, handleY, handleW, handleH, 2.0f);
        g.setColour(slateHighlight);
        g.drawRoundedRectangle(handleX, handleY, handleW, handleH, 2.0f, 1.0f);

        g.setColour(cyanGlow);
        g.fillRect(handleX + handleW * 0.5f - 1.0f, handleY + 2.0f, 2.0f, handleH - 4.0f);
    }
}
        g.fillRoundedRectangle(handleX, handleY, handleW, handleH, 2.0f);
        g.setColour(slateHighlight);
        g.drawRoundedRectangle(handleX, handleY, handleW, handleH, 2.0f, 1.0f);

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
    if (name.containsIgnoreCase("listen") || name.containsIgnoreCase("gate") || name.containsIgnoreCase("bleed"))
        activeColor = amberGlow;
    else if (name.equalsIgnoreCase("S"))
        activeColor = soloYellow;
    else if (name.equalsIgnoreCase("M"))
        activeColor = muteRed;
    else if (name.containsIgnoreCase("hit") || name.containsIgnoreCase("test"))
        activeColor = juce::Colour(0xff00ff88);
    else if (name.containsIgnoreCase("ø") || name.containsIgnoreCase("phase"))
        activeColor = phaseBlue;

    if (isToggled || shouldDrawButtonAsDown)
    {
        // Glowing Active / Depressed State
        g.setColour(activeColor.withAlpha(0.28f));
        g.fillRoundedRectangle(bounds, 3.5f);
        g.setColour(activeColor);
        g.drawRoundedRectangle(bounds, 3.5f, 1.4f);

        // Inner jewel glow
        g.setColour(activeColor.withAlpha(0.18f));
        g.fillRoundedRectangle(bounds.reduced(2.0f), 2.5f);
    }
    else
    {
        // Dark Tactile Brushed Hardware Button
        juce::Colour btnTop = shouldDrawButtonAsHighlighted ? juce::Colour(0xff3a414e) : juce::Colour(0xff282d38);
        juce::Colour btnBottom = shouldDrawButtonAsHighlighted ? juce::Colour(0xff242933) : juce::Colour(0xff181b22);

        juce::ColourGradient btnGrad(btnTop, bounds.getX(), bounds.getY(),
                                     btnBottom, bounds.getX(), bounds.getBottom(), false);
        g.setGradientFill(btnGrad);
        g.fillRoundedRectangle(bounds, 3.5f);

        g.setColour(slateBorder.brighter(0.2f));
        g.drawRoundedRectangle(bounds, 3.5f, 1.0f);
    }

    // Jewel LED indicator for toggleable buttons with no custom icon
    if (button.isToggleable() && !name.equalsIgnoreCase("S") && !name.equalsIgnoreCase("M") && !name.containsIgnoreCase("ø"))
    {
        float ledSize = 4.5f;
        float ledX = bounds.getX() + 5.0f;
        float ledY = bounds.getCentreY() - ledSize * 0.5f;

        if (isToggled)
        {
            g.setColour(activeColor.withAlpha(0.4f));
            g.fillEllipse(ledX - 1.5f, ledY - 1.5f, ledSize + 3.0f, ledSize + 3.0f);
            g.setColour(activeColor);
            g.fillEllipse(ledX, ledY, ledSize, ledSize);
            g.setColour(juce::Colours::white);
            g.fillEllipse(ledX + 1.0f, ledY + 1.0f, 1.5f, 1.5f);
        }
        else
        {
            g.setColour(juce::Colour(0xff12151b));
            g.fillEllipse(ledX, ledY, ledSize, ledSize);
            g.setColour(slateBorder);
            g.drawEllipse(ledX, ledY, ledSize, ledSize, 0.8f);
        }
    }
}

void SlateLookAndFeel::drawButtonText(juce::Graphics& g, juce::TextButton& button,
                                      bool /*shouldDrawButtonAsHighlighted*/,
                                      bool /*shouldDrawButtonAsDown*/)
{
    auto text = button.getButtonText();
    auto bounds = button.getLocalBounds().toFloat();
    bool isToggled = button.getToggleState();

    juce::Colour txtCol = isToggled ? textLight : textDim;
    if (isToggled)
    {
        if (text.equalsIgnoreCase("S")) txtCol = soloYellow;
        else if (text.equalsIgnoreCase("M")) txtCol = muteRed;
        else if (text.containsIgnoreCase("hit")) txtCol = juce::Colour(0xff00ff88);
    }

    g.setFont(juce::Font(10.5f, juce::Font::bold));
    g.setColour(txtCol);
    g.drawText(text, bounds.toNearestInt(), juce::Justification::centred);
}

void SlateLookAndFeel::drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
                                        bool shouldDrawButtonAsHighlighted,
                                        bool shouldDrawButtonAsDown)
{
    auto bounds = button.getLocalBounds().toFloat().reduced(1.0f);
    bool isToggled = button.getToggleState();
    auto name = button.getButtonText();

    if (name.equalsIgnoreCase("S") || name.equalsIgnoreCase("M") || name.containsIgnoreCase("ø") || name.containsIgnoreCase("phase"))
    {
        // Studio Channel Pushbutton Style
        juce::Colour activeColor = cyanGlow;
        if (name.equalsIgnoreCase("S")) activeColor = soloYellow;
        else if (name.equalsIgnoreCase("M")) activeColor = muteRed;
        else if (name.containsIgnoreCase("ø") || name.containsIgnoreCase("phase")) activeColor = phaseBlue;

        if (isToggled || shouldDrawButtonAsDown)
        {
            g.setColour(activeColor.withAlpha(0.28f));
            g.fillRoundedRectangle(bounds, 3.5f);
            g.setColour(activeColor);
            g.drawRoundedRectangle(bounds, 3.5f, 1.4f);

            g.setFont(juce::Font(11.0f, juce::Font::bold));
            g.setColour(activeColor);
            g.drawText(name, bounds.toNearestInt(), juce::Justification::centred);
        }
        else
        {
            juce::Colour btnTop = shouldDrawButtonAsHighlighted ? juce::Colour(0xff3a414e) : juce::Colour(0xff252933);
            juce::Colour btnBottom = shouldDrawButtonAsHighlighted ? juce::Colour(0xff222630) : juce::Colour(0xff161920);

            juce::ColourGradient btnGrad(btnTop, bounds.getX(), bounds.getY(),
                                         btnBottom, bounds.getX(), bounds.getBottom(), false);
            g.setGradientFill(btnGrad);
            g.fillRoundedRectangle(bounds, 3.5f);

            g.setColour(slateBorder);
            g.drawRoundedRectangle(bounds, 3.5f, 1.0f);

            g.setFont(juce::Font(11.0f, juce::Font::bold));
            g.setColour(textDim);
            g.drawText(name, bounds.toNearestInt(), juce::Justification::centred);
        }
        return;
    }

    // Default Hardware Toggle Switch with Jewel LED indicator
    float ledSize = 6.0f;
    float ledX = bounds.getX() + 3.0f;
    float ledY = bounds.getCentreY() - ledSize * 0.5f;

    juce::Colour activeColor = amberGlow;
    if (name.containsIgnoreCase("midi")) activeColor = cyanGlow;
    else if (name.containsIgnoreCase("gate")) activeColor = amberGlow;
    else if (name.containsIgnoreCase("bleed")) activeColor = amberGlow;

    if (isToggled)
    {
        g.setColour(activeColor.withAlpha(0.4f));
        g.fillEllipse(ledX - 2.0f, ledY - 2.0f, ledSize + 4.0f, ledSize + 4.0f);
        g.setColour(activeColor);
        g.fillEllipse(ledX, ledY, ledSize, ledSize);
        g.setColour(juce::Colours::white);
        g.fillEllipse(ledX + 1.2f, ledY + 1.2f, 1.8f, 1.8f);
    }
    else
    {
        g.setColour(juce::Colour(0xff0d0f14));
        g.fillEllipse(ledX, ledY, ledSize, ledSize);
        g.setColour(slateBorder);
        g.drawEllipse(ledX, ledY, ledSize, ledSize, 0.8f);
    }

    if (name.isNotEmpty())
    {
        g.setFont(juce::Font(9.5f, juce::Font::bold));
        g.setColour(isToggled ? textLight : textDim);
        auto textRect = bounds.withTrimmedLeft(ledSize + 7.0f);
        g.drawText(name, textRect.toNearestInt(), juce::Justification::centredLeft);
    }
}

void SlateLookAndFeel::drawComboBox(juce::Graphics& g, int width, int height, bool /*isButtonDown*/,
                                    int /*buttonX*/, int /*buttonY*/, int /*buttonW*/, int /*buttonH*/,
                                    juce::ComboBox& box)
{
    auto bounds = juce::Rectangle<float>(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height)).reduced(1.0f);

    // Recessed dark display well
    juce::ColourGradient bgGrad(juce::Colour(0xff12151c), bounds.getX(), bounds.getY(),
                                juce::Colour(0xff090b0f), bounds.getX(), bounds.getBottom(), false);
    g.setGradientFill(bgGrad);
    g.fillRoundedRectangle(bounds, 3.5f);

    g.setColour(box.hasKeyboardFocus(true) ? cyanGlow : slateBorder);
    g.drawRoundedRectangle(bounds, 3.5f, 1.0f);

    // Chevron down arrow
    const float arrowX = static_cast<float>(width) - 15.0f;
    const float arrowY = static_cast<float>(height) * 0.5f - 2.0f;
    juce::Path arrow;
    arrow.startNewSubPath(arrowX - 4.0f, arrowY);
    arrow.lineTo(arrowX, arrowY + 4.5f);
    arrow.lineTo(arrowX + 4.0f, arrowY);
    g.setColour(textDim);
    g.strokePath(arrow, juce::PathStrokeType(1.6f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
}

void SlateLookAndFeel::drawPopupMenuBackground(juce::Graphics& g, int width, int height)
{
    juce::Rectangle<float> bounds(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height));
    g.setColour(bgPanel);
    g.fillRoundedRectangle(bounds, 4.0f);
    g.setColour(slateBorder.brighter(0.2f));
    g.drawRoundedRectangle(bounds, 4.0f, 1.0f);
}

void SlateLookAndFeel::drawPopupMenuItem(juce::Graphics& g, const juce::Rectangle<int>& area,
                                         bool isSeparator, bool isActive, bool isHighlighted,
                                         bool /*isTicked*/, bool /*hasSubMenu*/,
                                         const juce::String& text, const juce::String& /*shortcutKeyText*/,
                                         const juce::Drawable* /*icon*/, const juce::Colour* textColour)
{
    if (isSeparator)
    {
        auto r = area.reduced(5, 0);
        g.setColour(slateBorder);
        g.fillRect(r.removeFromTop(1));
        return;
    }

    if (isHighlighted && isActive)
    {
        g.setColour(cyanGlow.withAlpha(0.22f));
        g.fillRect(area);
        g.setColour(cyanGlow);
        g.drawHorizontalLine(area.getY(), static_cast<float>(area.getX()), static_cast<float>(area.getRight()));
        g.drawHorizontalLine(area.getBottom() - 1, static_cast<float>(area.getX()), static_cast<float>(area.getRight()));
    }

    juce::Colour col = isActive ? (isHighlighted ? textLight : textDim.brighter(0.2f)) : textDim.darker(0.3f);
    if (textColour != nullptr) col = *textColour;

    g.setFont(juce::Font(11.0f, juce::Font::bold));
    g.setColour(col);

    auto textBounds = area.reduced(10, 0);
    g.drawText(text, textBounds, juce::Justification::centredLeft, true);
}

} // namespace yeahwaah
