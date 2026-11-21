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
const juce::Colour SlateLookAndFeel::textDim        = juce::Colour(0xff8896a8);
const juce::Colour SlateLookAndFeel::soloYellow     = juce::Colour(0xffffcc00);
const juce::Colour SlateLookAndFeel::muteRed        = juce::Colour(0xffff2a55);
const juce::Colour SlateLookAndFeel::phaseBlue      = juce::Colour(0xff00b0ff);

// SSL Console Colors
const juce::Colour SlateLookAndFeel::sslBlue        = juce::Colour(0xff1e88e5); // Filters / HF
const juce::Colour SlateLookAndFeel::sslGreen       = juce::Colour(0xff00c853); // LMF / Retrigger / Sensitivity
const juce::Colour SlateLookAndFeel::sslYellow      = juce::Colour(0xffffb300); // Solo / Range
const juce::Colour SlateLookAndFeel::sslRed         = juce::Colour(0xffe53935); // Dynamics / Master Out / Mute
const juce::Colour SlateLookAndFeel::sslAmber       = juce::Colour(0xffff9100); // Gate / Detail / Listen
const juce::Colour SlateLookAndFeel::sslCyan        = juce::Colour(0xff00e5ff); // Trigger 2 Electric Cyan / MIDI
const juce::Colour SlateLookAndFeel::sslWhite       = juce::Colour(0xffeaedf2); // Input Trim / Master Mix
const juce::Colour SlateLookAndFeel::sslDark        = juce::Colour(0xff101217); // Skirt / Bezel
const juce::Colour SlateLookAndFeel::sslPanel       = juce::Colour(0xff181c24); // Console Plate

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
    setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour(0xff0c0e13));
    setColour(juce::Slider::textBoxOutlineColourId, slateBorder);
    setColour(juce::ComboBox::backgroundColourId, bgControl);
    setColour(juce::ComboBox::textColourId, textLight);
    setColour(juce::ComboBox::outlineColourId, slateBorder);
    setColour(juce::PopupMenu::backgroundColourId, bgPanel);
    setColour(juce::PopupMenu::textColourId, textLight);
    setColour(juce::PopupMenu::highlightedBackgroundColourId, cyanGlow.withAlpha(0.28f));
}

juce::Font SlateLookAndFeel::getLabelFont(juce::Label& label)
{
    auto font = label.getFont();
    return font.boldened();
}

void SlateLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                                        float sliderPosProportional, float rotaryStartAngle,
                                        float rotaryEndAngle, juce::Slider& slider)
{
    const float radius = juce::jmin(static_cast<float>(width) * 0.5f, static_cast<float>(height) * 0.5f) - 2.0f;
    const float centreX = static_cast<float>(x) + static_cast<float>(width) * 0.5f;
    const float centreY = static_cast<float>(y) + static_cast<float>(height) * 0.5f;
    const float rx = centreX - radius;
    const float ry = centreY - radius;
    const float rw = radius * 2.0f;

    const float angle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);

    // Color theme based on SSL Console Module Architecture
    auto sName = slider.getName();
    juce::Colour knobCapColor = sslWhite;
    juce::Colour activeGlowColor = cyanGlow;

    if (sName.containsIgnoreCase("detail"))
    {
        knobCapColor = sslAmber;
        activeGlowColor = sslAmber;
    }
    else if (sName.containsIgnoreCase("gate") || sName.containsIgnoreCase("hold"))
    {
        knobCapColor = sslAmber;
        activeGlowColor = sslAmber;
    }
    else if (sName.containsIgnoreCase("cut") || sName.containsIgnoreCase("filter"))
    {
        knobCapColor = sslBlue;
        activeGlowColor = sslBlue;
    }
    else if (sName.containsIgnoreCase("sensitivity") || sName.containsIgnoreCase("retrigger"))
    {
        knobCapColor = sslGreen;
        activeGlowColor = sslGreen;
    }
    else if (sName.containsIgnoreCase("dynamics") || sName.containsIgnoreCase("attack"))
    {
        knobCapColor = sslRed;
        activeGlowColor = sslRed;
    }
    else if (sName.containsIgnoreCase("release"))
    {
        knobCapColor = sslGreen;
        activeGlowColor = sslGreen;
    }
    else if (sName.containsIgnoreCase("output"))
    {
        knobCapColor = sslRed;
        activeGlowColor = sslRed;
    }
    else if (sName.containsIgnoreCase("mix"))
    {
        knobCapColor = sslCyan;
        activeGlowColor = sslCyan;
    }
    else if (sName.containsIgnoreCase("pan"))
    {
        knobCapColor = juce::Colour(0xff4a5568);
        activeGlowColor = cyanGlow;
    }
    else if (sName.containsIgnoreCase("tune"))
    {
        knobCapColor = sslAmber;
        activeGlowColor = sslAmber;
    }
    else if (sName.containsIgnoreCase("leakage") || sName.containsIgnoreCase("bleed"))
    {
        knobCapColor = sslAmber;
        activeGlowColor = sslAmber;
    }
    else if (sName.containsIgnoreCase("midi"))
    {
        knobCapColor = sslCyan;
        activeGlowColor = sslCyan;
    }

    // 1. Recessed Outer Bezel Socket with Drop Shadow (SSL Console Socket Well)
    g.setColour(juce::Colour(0xff050608));
    g.fillEllipse(rx - 2.5f, ry - 2.5f, rw + 5.0f, rw + 5.0f);
    g.setColour(slateBorder.darker(0.2f));
    g.drawEllipse(rx - 2.0f, ry - 2.0f, rw + 4.0f, rw + 4.0f, 1.2f);

    // 2. Peripheral SSL Calibration Hash Marks / Tick Scale around the knob
    const int numTicks = (radius > 24.0f) ? 17 : 11;
    const float tickInnerRadius = radius + 0.5f;
    const float tickOuterRadius = radius + 2.8f;

    for (int i = 0; i < numTicks; ++i)
    {
        float tickFrac = static_cast<float>(i) / static_cast<float>(numTicks - 1);
        float tickAngle = rotaryStartAngle + tickFrac * (rotaryEndAngle - rotaryStartAngle);
        float sinA = std::sin(tickAngle);
        float cosA = std::cos(tickAngle);

        float p1x = centreX + sinA * tickInnerRadius;
        float p1y = centreY - cosA * tickInnerRadius;
        float p2x = centreX + sinA * tickOuterRadius;
        float p2y = centreY - cosA * tickOuterRadius;

        bool isPassed = (tickFrac <= sliderPosProportional + 0.02f);
        if (isPassed && sliderPosProportional > 0.01f)
        {
            g.setColour(activeGlowColor.withAlpha(0.9f));
            g.drawLine(p1x, p1y, p2x, p2y, 1.5f);
        }
        else
        {
            g.setColour(slateBorder.brighter(0.2f));
            g.drawLine(p1x, p1y, p2x, p2y, 1.0f);
        }
    }

    // 3. Recessed Background Arc Track
    const float arcRadius = radius - 3.0f;
    juce::Path backgroundArc;
    backgroundArc.addCentredArc(centreX, centreY, arcRadius, arcRadius, 0.0f,
                                rotaryStartAngle, rotaryEndAngle, true);
    g.setColour(juce::Colour(0xff07080b));
    g.strokePath(backgroundArc, juce::PathStrokeType(5.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    g.setColour(slateBorder.darker(0.4f));
    g.strokePath(backgroundArc, juce::PathStrokeType(1.0f));

    // 4. Glowing Active Arc Track
    if (sliderPosProportional > 0.005f)
    {
        juce::Path activeArc;
        activeArc.addCentredArc(centreX, centreY, arcRadius, arcRadius, 0.0f,
                                rotaryStartAngle, angle, true);
        // Outer halo bloom
        g.setColour(activeGlowColor.withAlpha(0.38f));
        g.strokePath(activeArc, juce::PathStrokeType(7.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        // Solid core arc
        g.setColour(activeGlowColor);
        g.strokePath(activeArc, juce::PathStrokeType(3.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }

    // 5. Machined SSL Ribbed Aluminum Knob Base / Skirt
    const float knobRadius = radius - 5.5f;
    const float kx = centreX - knobRadius;
    const float ky = centreY - knobRadius;
    const float kw = knobRadius * 2.0f;

    // Dark knurled skirt ring
    g.setColour(juce::Colour(0xff0b0d11));
    g.fillEllipse(kx - 1.5f, ky - 1.5f, kw + 3.0f, kw + 3.0f);

    // SSL Console Knurled Ribs around the perimeter
    const int knurlSteps = (knobRadius > 20.0f) ? 28 : 20;
    for (int k = 0; k < knurlSteps; ++k)
    {
        float a = static_cast<float>(k) * (juce::MathConstants<float>::twoPi / static_cast<float>(knurlSteps));
        float p1x = centreX + std::sin(a) * (knobRadius + 0.2f);
        float p1y = centreY - std::cos(a) * (knobRadius + 0.2f);
        float p2x = centreX + std::sin(a) * (knobRadius + 2.0f);
        float p2y = centreY - std::cos(a) * (knobRadius + 2.0f);
        g.setColour(juce::Colour(0xff333b49));
        g.drawLine(p1x, p1y, p2x, p2y, 1.1f);
    }

    // Brushed dark metallic base
    juce::ColourGradient baseGrad(juce::Colour(0xff3c4554), centreX - knobRadius * 0.45f, centreY - knobRadius * 0.45f,
                                  juce::Colour(0xff161920), centreX + knobRadius * 0.65f, centreY + knobRadius * 0.65f, true);
    g.setGradientFill(baseGrad);
    g.fillEllipse(kx, ky, kw, kw);
    g.setColour(juce::Colour(0xff556277));
    g.drawEllipse(kx, ky, kw, kw, 1.2f);

    // 6. SSL Iconic Colored Cap on Top
    const float capRadius = knobRadius * 0.72f;
    const float cx = centreX - capRadius;
    const float cy = centreY - capRadius;
    const float cw = capRadius * 2.0f;

    // Cap drop shadow onto skirt
    g.setColour(juce::Colour(0xff06070a).withAlpha(0.85f));
    g.fillEllipse(cx - 1.0f, cy + 1.0f, cw + 2.0f, cw + 2.0f);

    // SSL Anodized Colored Top Cap Gradient
    juce::Colour capHighlight = knobCapColor.brighter(0.35f);
    juce::Colour capShadow = knobCapColor.darker(0.55f);
    juce::ColourGradient capGrad(capHighlight, centreX - capRadius * 0.35f, centreY - capRadius * 0.35f,
                                 capShadow, centreX + capRadius * 0.55f, centreY + capRadius * 0.55f, true);
    g.setGradientFill(capGrad);
    g.fillEllipse(cx, cy, cw, cw);

    // Polished cap rim
    g.setColour(knobCapColor.brighter(0.6f).withAlpha(0.85f));
    g.drawEllipse(cx, cy, cw, cw, 1.0f);

    // Center jewel pip
    g.setColour(juce::Colour(0xff14171d));
    g.fillEllipse(centreX - 3.0f, centreY - 3.0f, 6.0f, 6.0f);

    // 7. High-Intensity SSL Illuminated Pointer Needle
    const float indicatorStart = 3.5f;
    const float indicatorLen = knobRadius + 1.0f;
    juce::Point<float> p1(centreX + std::sin(angle) * indicatorStart,
                          centreY - std::cos(angle) * indicatorStart);
    juce::Point<float> p2(centreX + std::sin(angle) * indicatorLen,
                          centreY - std::cos(angle) * indicatorLen);

    // Pointer bloom halo
    g.setColour(activeGlowColor.withAlpha(0.65f));
    g.drawLine(p1.x, p1.y, p2.x, p2.y, 5.5f);

    // Pointer core line
    g.setColour(juce::Colours::white);
    g.drawLine(p1.x, p1.y, p2.x, p2.y, 2.5f);
}

void SlateLookAndFeel::drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
                                        float sliderPos, float /*minSliderPos*/, float /*maxSliderPos*/,
                                        const juce::Slider::SliderStyle /*style*/, juce::Slider& slider)
{
    const bool isVertical = slider.isVertical();

    if (isVertical)
    {
        const float trackWidth = 8.0f;
        float trackX = static_cast<float>(x) + static_cast<float>(width) * 0.5f - trackWidth * 0.5f;

        // Recessed Console Fader Well with dark felt depth
        g.setColour(juce::Colour(0xff06070a));
        g.fillRoundedRectangle(trackX, static_cast<float>(y), trackWidth, static_cast<float>(height), 3.5f);
        g.setColour(slateBorder.darker(0.35f));
        g.drawRoundedRectangle(trackX, static_cast<float>(y), trackWidth, static_cast<float>(height), 3.5f, 1.2f);

        // Center guide slit & silver rail
        g.setColour(juce::Colour(0xff020204));
        g.drawVerticalLine(static_cast<int>(trackX + trackWidth * 0.5f), static_cast<float>(y) + 2.0f, static_cast<float>(y + height) - 2.0f);
        g.setColour(juce::Colour(0xff3a4352));
        g.drawVerticalLine(static_cast<int>(trackX + trackWidth * 0.5f) + 1, static_cast<float>(y) + 4.0f, static_cast<float>(y + height) - 4.0f);

        // Calibrated SSL dB scale tick marks flanking the fader track
        g.setColour(slateBorder.brighter(0.25f));
        const int numTicks = 9;
        for (int t = 0; t < numTicks; ++t)
        {
            float frac = static_cast<float>(t) / static_cast<float>(numTicks - 1);
            float ty = static_cast<float>(y) + 12.0f + frac * (static_cast<float>(height) - 24.0f);

            // Bold tick for 0 dB unity mark (around 30% from top in standard gain law)
            bool isUnity = (t == 2);
            float tickLen = isUnity ? 7.0f : 4.0f;
            if (isUnity)
                g.setColour(cyanGlow.withAlpha(0.85f));
            else
                g.setColour(slateBorder.brighter(0.2f));

            g.drawHorizontalLine(static_cast<int>(ty), trackX - tickLen - 1.0f, trackX - 1.0f);
            g.drawHorizontalLine(static_cast<int>(ty), trackX + trackWidth + 1.0f, trackX + trackWidth + tickLen + 1.0f);
        }

        // Active cyan/amber level fill track below fader thumb
        float fillY = sliderPos;
        float fillH = static_cast<float>(y + height) - sliderPos;
        if (fillH > 2.0f)
        {
            g.setColour(cyanGlow.withAlpha(0.6f));
            g.fillRoundedRectangle(trackX + 1.5f, fillY, trackWidth - 3.0f, fillH, 2.0f);
        }

        // Chunky SSL Console Fader Cap (Alps / Penny & Giles style)
        float handleW = static_cast<float>(width) - 6.0f;
        float handleH = 26.0f;
        float handleX = static_cast<float>(x) + 3.0f;
        float handleY = sliderPos - handleH * 0.5f;

        // Heavy drop shadow underneath cap
        g.setColour(juce::Colour(0xff030406).withAlpha(0.88f));
        g.fillRoundedRectangle(handleX - 1.0f, handleY + 4.0f, handleW + 2.0f, handleH, 3.5f);

        // Stepped metallic body gradient (classic SSL charcoal/silver console fader)
        juce::ColourGradient capGrad(juce::Colour(0xff4a5465), handleX, handleY,
                                     juce::Colour(0xff181b21), handleX, handleY + handleH, false);
        g.setGradientFill(capGrad);
        g.fillRoundedRectangle(handleX, handleY, handleW, handleH, 3.0f);

        // Polished chamfer highlights
        g.setColour(juce::Colour(0xff74849e));
        g.drawRoundedRectangle(handleX, handleY, handleW, handleH, 3.0f, 1.2f);

        // Center concave finger rest depression
        juce::Rectangle<float> trough(handleX + 3.0f, handleY + 6.0f, handleW - 6.0f, handleH - 12.0f);
        g.setColour(juce::Colour(0xff12151c));
        g.fillRoundedRectangle(trough, 2.0f);

        // Tactile knurled ridges on upper and lower lips
        g.setColour(juce::Colour(0xff2d3543));
        g.drawHorizontalLine(static_cast<int>(handleY + 3.0f), handleX + 4.0f, handleX + handleW - 4.0f);
        g.drawHorizontalLine(static_cast<int>(handleY + handleH - 4.0f), handleX + 4.0f, handleX + handleW - 4.0f);

        // High-contrast illuminated center pointer stripe
        g.setColour(cyanGlow.withAlpha(0.75f));
        g.fillRect(handleX + 3.0f, handleY + handleH * 0.5f - 1.5f, handleW - 6.0f, 3.0f);
        g.setColour(juce::Colours::white);
        g.fillRect(handleX + 4.0f, handleY + handleH * 0.5f - 0.5f, handleW - 8.0f, 1.0f);
    }
    else
    {
        // Horizontal tactile mini slider (Envelope Attack/Decay, Velocity Limiters)
        const float trackH = 7.0f;
        float trackY = static_cast<float>(y) + static_cast<float>(height) * 0.5f - trackH * 0.5f;

        // Recessed dark well
        g.setColour(juce::Colour(0xff090b0f));
        g.fillRoundedRectangle(static_cast<float>(x), trackY, static_cast<float>(width), trackH, 3.0f);
        g.setColour(slateBorder);
        g.drawRoundedRectangle(static_cast<float>(x), trackY, static_cast<float>(width), trackH, 3.0f, 1.0f);

        // Active fill bar
        float fillW = std::max(0.0f, sliderPos - static_cast<float>(x));
        g.setColour(cyanGlow.withAlpha(0.65f));
        g.fillRoundedRectangle(static_cast<float>(x) + 1.0f, trackY + 1.0f, fillW, trackH - 2.0f, 2.0f);

        // Chunky tactile slider thumb
        float thumbW = 16.0f;
        float thumbH = static_cast<float>(height) - 4.0f;
        float thumbX = sliderPos - thumbW * 0.5f;
        float thumbY = static_cast<float>(y) + 2.0f;

        // Drop shadow
        g.setColour(juce::Colour(0xff050608).withAlpha(0.8f));
        g.fillRoundedRectangle(thumbX, thumbY + 1.5f, thumbW, thumbH, 2.5f);

        // Metallic body
        juce::ColourGradient thumbGrad(juce::Colour(0xff4c576b), thumbX, thumbY,
                                       juce::Colour(0xff1d212b), thumbX, thumbY + thumbH, false);
        g.setGradientFill(thumbGrad);
        g.fillRoundedRectangle(thumbX, thumbY, thumbW, thumbH, 2.5f);
        g.setColour(slateHighlight);
        g.drawRoundedRectangle(thumbX, thumbY, thumbW, thumbH, 2.5f, 1.0f);

        // Center pointer pip
        g.setColour(cyanGlow);
        g.fillRect(thumbX + thumbW * 0.5f - 1.0f, thumbY + 3.0f, 2.0f, thumbH - 6.0f);
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
    if (name.containsIgnoreCase("listen") || name.containsIgnoreCase("gate") || name.containsIgnoreCase("bleed") || name.containsIgnoreCase("audition"))
        activeColor = amberGlow;
    else if (name.equalsIgnoreCase("S") || name.containsIgnoreCase("solo"))
        activeColor = soloYellow;
    else if (name.equalsIgnoreCase("M") || name.containsIgnoreCase("mute"))
        activeColor = muteRed;
    else if (name.containsIgnoreCase("hit") || name.containsIgnoreCase("test"))
        activeColor = juce::Colour(0xff00e676);
    else if (name.contains(juce::CharPointer_UTF8("\xc3\x98")) || name.contains(juce::CharPointer_UTF8("\xc3\xb8")) || name.containsIgnoreCase("phase") || name.equalsIgnoreCase("inv"))
        activeColor = phaseBlue;
    else if (name.containsIgnoreCase("on"))
        activeColor = juce::Colour(0xff00e676);

    // Beveled SSL Console Pushbutton Frame
    if (isToggled || shouldDrawButtonAsDown)
    {
        // Fully backlit illuminated button
        g.setColour(activeColor.withAlpha(0.32f));
        g.fillRoundedRectangle(bounds, 3.5f);
        g.setColour(activeColor);
        g.drawRoundedRectangle(bounds, 3.5f, 1.4f);

        // Inner glowing core
        g.setColour(activeColor.withAlpha(0.18f));
        g.fillRoundedRectangle(bounds.reduced(2.0f), 2.5f);
    }
    else
    {
        // Dark tactile hardware button (unlit)
        juce::Colour btnTop = shouldDrawButtonAsHighlighted ? juce::Colour(0xff363e4d) : juce::Colour(0xff242934);
        juce::Colour btnBottom = shouldDrawButtonAsHighlighted ? juce::Colour(0xff20252f) : juce::Colour(0xff161921);

        juce::ColourGradient btnGrad(btnTop, bounds.getX(), bounds.getY(),
                                     btnBottom, bounds.getX(), bounds.getBottom(), false);
        g.setGradientFill(btnGrad);
        g.fillRoundedRectangle(bounds, 3.5f);

        g.setColour(slateBorder.brighter(0.25f));
        g.drawRoundedRectangle(bounds, 3.5f, 1.0f);
    }

    // Jewel LED indicator lens on pushbuttons that benefit from it
    if (button.isToggleable() && !name.equalsIgnoreCase("S") && !name.equalsIgnoreCase("M") && !name.containsIgnoreCase("ø"))
    {
        float ledSize = 5.0f;
        float ledX = bounds.getX() + 6.0f;
        float ledY = bounds.getCentreY() - ledSize * 0.5f;

        if (isToggled)
        {
            g.setColour(activeColor.withAlpha(0.45f));
            g.fillEllipse(ledX - 1.5f, ledY - 1.5f, ledSize + 3.0f, ledSize + 3.0f);
            g.setColour(activeColor);
            g.fillEllipse(ledX, ledY, ledSize, ledSize);
            g.setColour(juce::Colours::white);
            g.fillEllipse(ledX + 1.2f, ledY + 1.2f, 1.8f, 1.8f);
        }
        else
        {
            g.setColour(juce::Colour(0xff0d1016));
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
        else if (text.containsIgnoreCase("on")) txtCol = juce::Colour(0xff00ffaa);
    }

    g.setFont(juce::Font(11.0f, juce::Font::bold));
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

    // Studio pushbuttons: Solo, Mute, Phase, or Slot Channel On/Off
    juce::Colour activeColor = cyanGlow;
    if (name.equalsIgnoreCase("S") || name.containsIgnoreCase("solo"))
        activeColor = soloYellow;
    else if (name.equalsIgnoreCase("M") || name.containsIgnoreCase("mute"))
        activeColor = muteRed;
    else if (name.contains(juce::CharPointer_UTF8("\xc3\x98")) || name.contains(juce::CharPointer_UTF8("\xc3\xb8")) || name.containsIgnoreCase("phase") || name.equalsIgnoreCase("inv"))
        activeColor = phaseBlue;
    else if (name.containsIgnoreCase("on") || name.containsIgnoreCase("ch on") || name.containsIgnoreCase("enable"))
        activeColor = juce::Colour(0xff00e676);
    else if (name.containsIgnoreCase("gate") || name.containsIgnoreCase("bleed"))
        activeColor = amberGlow;

    if (isToggled || shouldDrawButtonAsDown)
    {
        // Active illuminated button (SSL Backlit Studio Pushbutton)
        g.setColour(activeColor.withAlpha(0.36f));
        g.fillRoundedRectangle(bounds, 3.5f);
        g.setColour(activeColor);
        g.drawRoundedRectangle(bounds, 3.5f, 1.4f);

        // Core glow
        g.setColour(activeColor.withAlpha(0.24f));
        g.fillRoundedRectangle(bounds.reduced(2.0f), 2.5f);

        g.setFont(juce::Font(11.5f, juce::Font::bold));
        juce::Colour txtCol = (name.equalsIgnoreCase("S")) ? juce::Colours::black : juce::Colours::white;
        g.setColour(txtCol);
        g.drawText(name, bounds.toNearestInt(), juce::Justification::centred);
    }
    else
    {
        // Dark tactile unlit button
        juce::Colour btnTop = shouldDrawButtonAsHighlighted ? juce::Colour(0xff363e4d) : juce::Colour(0xff242934);
        juce::Colour btnBottom = shouldDrawButtonAsHighlighted ? juce::Colour(0xff20252f) : juce::Colour(0xff161921);

        juce::ColourGradient btnGrad(btnTop, bounds.getX(), bounds.getY(),
                                     btnBottom, bounds.getX(), bounds.getBottom(), false);
        g.setGradientFill(btnGrad);
        g.fillRoundedRectangle(bounds, 3.5f);

        g.setColour(slateBorder);
        g.drawRoundedRectangle(bounds, 3.5f, 1.0f);

        juce::Colour unlitCol = textDim.brighter(0.2f);
        if (name.equalsIgnoreCase("S")) unlitCol = soloYellow.withAlpha(0.9f);
        else if (name.equalsIgnoreCase("M")) unlitCol = muteRed.withAlpha(0.9f);
        else if (name.contains(juce::CharPointer_UTF8("\xc3\x98")) || name.contains(juce::CharPointer_UTF8("\xc3\xb8")) || name.containsIgnoreCase("phase")) unlitCol = phaseBlue.withAlpha(0.9f);

        g.setFont(juce::Font(11.5f, juce::Font::bold));
        g.setColour(unlitCol);
        g.drawText(name, bounds.toNearestInt(), juce::Justification::centred);
    }
}

void SlateLookAndFeel::drawComboBox(juce::Graphics& g, int width, int height, bool /*isButtonDown*/,
                                    int /*buttonX*/, int /*buttonY*/, int /*buttonW*/, int /*buttonH*/,
                                    juce::ComboBox& box)
{
    auto bounds = juce::Rectangle<float>(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height)).reduced(1.0f);

    juce::ColourGradient bgGrad(juce::Colour(0xff141822), bounds.getX(), bounds.getY(),
                                juce::Colour(0xff0a0c11), bounds.getX(), bounds.getBottom(), false);
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
    g.setColour(textLight);
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
        g.setColour(cyanGlow.withAlpha(0.25f));
        g.fillRect(area);
        g.setColour(cyanGlow);
        g.drawHorizontalLine(area.getY(), static_cast<float>(area.getX()), static_cast<float>(area.getRight()));
        g.drawHorizontalLine(area.getBottom() - 1, static_cast<float>(area.getX()), static_cast<float>(area.getRight()));
    }

    juce::Colour col = isActive ? (isHighlighted ? textLight : textDim.brighter(0.2f)) : textDim.darker(0.3f);
    if (textColour != nullptr) col = *textColour;

    g.setFont(juce::Font(11.5f, juce::Font::bold));
    g.setColour(col);

    auto textBounds = area.reduced(10, 0);
    g.drawText(text, textBounds, juce::Justification::centredLeft, true);
}

} // namespace yeahwaah
