#include "SlateLookAndFeel.h"
#include <cmath>

namespace yeahwaah
{

const juce::Colour SlateLookAndFeel::bgDark         = juce::Colour(0xff161512);
const juce::Colour SlateLookAndFeel::bgPanel        = juce::Colour(0xff1f1d19);
const juce::Colour SlateLookAndFeel::bgControl      = juce::Colour(0xff2a2823);
const juce::Colour SlateLookAndFeel::cyanGlow       = juce::Colour(0xffe9b44c);
const juce::Colour SlateLookAndFeel::amberGlow      = juce::Colour(0xfff7b733);
const juce::Colour SlateLookAndFeel::slateBorder    = juce::Colour(0xff3d392f);
const juce::Colour SlateLookAndFeel::slateHighlight = juce::Colour(0xff5e584a);
const juce::Colour SlateLookAndFeel::textLight      = juce::Colour(0xffe9dfc6);
const juce::Colour SlateLookAndFeel::textDim        = juce::Colour(0xff9d927a);
const juce::Colour SlateLookAndFeel::soloYellow     = juce::Colour(0xfff7b733);
const juce::Colour SlateLookAndFeel::muteRed        = juce::Colour(0xffd9482b);
const juce::Colour SlateLookAndFeel::phaseBlue      = juce::Colour(0xfff7b733);
const juce::Colour SlateLookAndFeel::titleAmber     = juce::Colour(0xffd9a24a);
const juce::Colour SlateLookAndFeel::meterGreen     = juce::Colour(0xff8fbf4a);

const juce::Colour SlateLookAndFeel::sslBlue        = juce::Colour(0xff5b8fd8);
const juce::Colour SlateLookAndFeel::sslGreen       = juce::Colour(0xff7bb552);
const juce::Colour SlateLookAndFeel::sslYellow      = juce::Colour(0xfff2b53a);
const juce::Colour SlateLookAndFeel::sslRed         = juce::Colour(0xffd9482b);
const juce::Colour SlateLookAndFeel::sslAmber       = juce::Colour(0xfff7b733);
const juce::Colour SlateLookAndFeel::sslCyan        = juce::Colour(0xff3fb8a8);
const juce::Colour SlateLookAndFeel::sslWhite       = juce::Colour(0xffe9dfc6);
const juce::Colour SlateLookAndFeel::sslDark        = juce::Colour(0xff161512);
const juce::Colour SlateLookAndFeel::sslPanel       = juce::Colour(0xff1f1d19);

juce::Colour SlateLookAndFeel::getSlotColor(int slotIndex)
{
    static const juce::Colour colors[] = {
        juce::Colour(0xfff2b53a), // 1 kick: amber
        juce::Colour(0xff5b8fd8), // 2 snare: blue
        juce::Colour(0xff7bb552), // 3 rim: green
        juce::Colour(0xffa46bd0), // 4 room: purple
        juce::Colour(0xffe8793a), // 5 wires: orange
        juce::Colour(0xff3fb8a8)  // 6 aux: teal
    };
    return colors[juce::jlimit(0, 5, slotIndex)];
}

juce::Colour SlateLookAndFeel::getSlotWaveColor(int slotIndex)
{
    static const juce::Colour colors[] = {
        juce::Colour(0xfff0b030), juce::Colour(0xfff0a838), juce::Colour(0xffe9b050),
        juce::Colour(0xffd88aa8), juce::Colour(0xfff09a30), juce::Colour(0xff45c0b0)
    };
    return colors[juce::jlimit(0, 5, slotIndex)];
}

juce::Font SlateLookAndFeel::boldFont(float height)
{
    return juce::Font(height, juce::Font::bold);
}

SlateLookAndFeel::SlateLookAndFeel()
{
    setColour(juce::ResizableWindow::backgroundColourId, bgDark);
    setColour(juce::Slider::textBoxTextColourId, textLight);
    setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour(0xff0e0d0b));
    setColour(juce::Slider::textBoxOutlineColourId, slateBorder);
    setColour(juce::ComboBox::backgroundColourId, bgControl);
    setColour(juce::ComboBox::textColourId, textLight);
    setColour(juce::ComboBox::outlineColourId, slateBorder);
    setColour(juce::PopupMenu::backgroundColourId, bgPanel);
    setColour(juce::PopupMenu::textColourId, textLight);
    setColour(juce::PopupMenu::highlightedBackgroundColourId, amberGlow.withAlpha(0.28f));
    setColour(juce::Label::textColourId, textLight);
    setColour(juce::TooltipWindow::backgroundColourId, juce::Colour(0xff0e0d0b));
    setColour(juce::TooltipWindow::textColourId, textLight);
    setColour(juce::TooltipWindow::outlineColourId, slateBorder);
}

juce::Font SlateLookAndFeel::getLabelFont(juce::Label& label)
{
    return label.getFont();
}

void SlateLookAndFeel::drawLabel(juce::Graphics& g, juce::Label& label)
{
    auto bounds = label.getLocalBounds().toFloat();
    const bool isSliderText = dynamic_cast<juce::Slider*>(label.getParentComponent()) != nullptr;

    if (isSliderText)
    {
        // Recessed engraved readout below knobs
        g.setColour(juce::Colour(0xff0e0d0b));
        g.fillRoundedRectangle(bounds.reduced(0.5f), 4.0f);
        g.setColour(juce::Colour(0xff000000).withAlpha(0.6f));
        g.drawRoundedRectangle(bounds.reduced(0.5f), 4.0f, 1.2f);
        g.setColour(juce::Colour(0xff4a463a).withAlpha(0.7f));
        g.drawHorizontalLine(static_cast<int>(bounds.getBottom()) - 1, bounds.getX() + 4.0f, bounds.getRight() - 4.0f);

        g.setColour(textLight);
        g.setFont(boldFont(juce::jmin(17.0f, bounds.getHeight() * 0.72f)));
        g.drawText(label.getText(), bounds.toNearestInt(), juce::Justification::centred, false);
        return;
    }

    if (label.isOpaque() || label.findColour(juce::Label::backgroundColourId).isOpaque())
        g.fillAll(label.findColour(juce::Label::backgroundColourId));

    g.setColour(label.findColour(juce::Label::textColourId));
    g.setFont(label.getFont());
    g.drawText(label.getText(), label.getLocalBounds().reduced(2, 0), label.getJustificationType(), true);
}

void SlateLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                                        float sliderPosProportional, float rotaryStartAngle,
                                        float rotaryEndAngle, juce::Slider& /*slider*/)
{
    const float half = juce::jmin(static_cast<float>(width), static_cast<float>(height)) * 0.5f;
    const float cx = static_cast<float>(x) + static_cast<float>(width) * 0.5f;
    const float cy = static_cast<float>(y) + static_cast<float>(height) * 0.5f;

    const float bodyR = half * 0.70f;
    const float angle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);

    auto polar = [&](float r, float a) {
        return juce::Point<float>(cx + r * std::sin(a), cy - r * std::cos(a));
    };

    // Scale tick marks
    const int numTicks = 11;
    for (int i = 0; i < numTicks; ++i)
    {
        const float a = rotaryStartAngle + (static_cast<float>(i) / static_cast<float>(numTicks - 1)) * (rotaryEndAngle - rotaryStartAngle);
        const bool major = (i == 0 || i == numTicks - 1 || i == numTicks / 2);
        g.setColour(textLight.withAlpha(major ? 0.85f : 0.55f));
        g.drawLine(juce::Line<float>(polar(bodyR + half * 0.10f, a), polar(bodyR + half * (major ? 0.27f : 0.22f), a)),
                   major ? 2.2f : 1.6f);
    }

    // Soft drop shadow
    g.setColour(juce::Colours::black.withAlpha(0.55f));
    g.fillEllipse(cx - bodyR + 2.0f, cy - bodyR + 4.0f, bodyR * 2.0f, bodyR * 2.0f);

    // Knurled skirt
    juce::ColourGradient skirt(juce::Colour(0xff4a463e), cx - bodyR * 0.6f, cy - bodyR * 0.8f,
                               juce::Colour(0xff0c0b09), cx + bodyR * 0.7f, cy + bodyR * 0.9f, false);
    g.setGradientFill(skirt);
    g.fillEllipse(cx - bodyR, cy - bodyR, bodyR * 2.0f, bodyR * 2.0f);

    g.setColour(juce::Colour(0xff000000).withAlpha(0.45f));
    const int knurls = 40;
    for (int i = 0; i < knurls; ++i)
    {
        const float a = juce::MathConstants<float>::twoPi * static_cast<float>(i) / static_cast<float>(knurls);
        g.drawLine(juce::Line<float>(polar(bodyR * 0.90f, a), polar(bodyR * 0.99f, a)), 1.2f);
    }

    // Top cap
    const float capR = bodyR * 0.80f;
    juce::ColourGradient cap(juce::Colour(0xff6a655a), cx - capR * 0.5f, cy - capR * 0.7f,
                             juce::Colour(0xff1c1b17), cx + capR * 0.6f, cy + capR * 0.8f, true);
    g.setGradientFill(cap);
    g.fillEllipse(cx - capR, cy - capR, capR * 2.0f, capR * 2.0f);
    g.setColour(juce::Colour(0xff0a0907));
    g.drawEllipse(cx - capR, cy - capR, capR * 2.0f, capR * 2.0f, 1.4f);
    g.setColour(juce::Colour(0xff7a7568).withAlpha(0.35f));
    g.drawEllipse(cx - capR + 1.5f, cy - capR + 1.5f, capR * 2.0f - 3.0f, capR * 2.0f - 3.0f, 1.0f);

    // Pointer
    const float pw = juce::jmax(2.4f, half * 0.075f);
    g.setColour(juce::Colours::black.withAlpha(0.5f));
    g.drawLine(juce::Line<float>(polar(capR * 0.18f, angle) + juce::Point<float>(1.0f, 1.5f),
                                 polar(capR * 0.94f, angle) + juce::Point<float>(1.0f, 1.5f)), pw + 1.0f);
    g.setColour(textLight);
    g.drawLine(juce::Line<float>(polar(capR * 0.18f, angle), polar(capR * 0.94f, angle)), pw);
}

void SlateLookAndFeel::drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
                                        float sliderPos, float /*minSliderPos*/, float /*maxSliderPos*/,
                                        const juce::Slider::SliderStyle style, juce::Slider& slider)
{
    const bool horizontal = (style == juce::Slider::LinearHorizontal || style == juce::Slider::LinearBar);
    juce::Rectangle<float> area(static_cast<float>(x), static_cast<float>(y), static_cast<float>(width), static_cast<float>(height));

    if (horizontal)
    {
        const float trackH = juce::jmin(8.0f, area.getHeight() * 0.5f);
        juce::Rectangle<float> track(area.getX(), area.getCentreY() - trackH * 0.5f, area.getWidth(), trackH);

        g.setColour(juce::Colour(0xff0a0907));
        g.fillRoundedRectangle(track, trackH * 0.5f);
        g.setColour(juce::Colour(0xff4a463a).withAlpha(0.8f));
        g.drawRoundedRectangle(track, trackH * 0.5f, 1.0f);

        juce::Rectangle<float> fill = track.withRight(juce::jlimit(track.getX(), track.getRight(), sliderPos)).reduced(1.0f);
        if (fill.getWidth() > 1.0f)
        {
            juce::ColourGradient grad(juce::Colour(0xfff9d27a), fill.getX(), fill.getY(),
                                      juce::Colour(0xffc98a1a), fill.getX(), fill.getBottom(), false);
            g.setGradientFill(grad);
            g.fillRoundedRectangle(fill, trackH * 0.5f);
        }

        const float tx = juce::jlimit(track.getX(), track.getRight(), sliderPos);
        g.setColour(juce::Colour(0xff000000).withAlpha(0.5f));
        g.fillRoundedRectangle(tx - 3.5f, area.getCentreY() - 8.0f + 1.5f, 7.0f, 16.0f, 2.0f);
        g.setColour(textLight);
        g.fillRoundedRectangle(tx - 3.5f, area.getCentreY() - 8.0f, 7.0f, 16.0f, 2.0f);
    }
    else
    {
        const float trackW = 8.0f;
        juce::Rectangle<float> track(area.getCentreX() - trackW * 0.5f, area.getY(), trackW, area.getHeight());
        g.setColour(juce::Colour(0xff0a0907));
        g.fillRoundedRectangle(track, 4.0f);
        g.setColour(textLight);
        g.fillRoundedRectangle(area.getCentreX() - 9.0f, sliderPos - 4.0f, 18.0f, 8.0f, 2.0f);
    }

    juce::ignoreUnused(slider);
}

void SlateLookAndFeel::drawButtonBackground(juce::Graphics& g, juce::Button& button,
                                            const juce::Colour& /*backgroundColour*/,
                                            bool shouldDrawButtonAsHighlighted,
                                            bool shouldDrawButtonAsDown)
{
    auto bounds = button.getLocalBounds().toFloat().reduced(1.0f);
    const bool lit = button.getToggleState() || shouldDrawButtonAsDown;

    juce::Colour top = shouldDrawButtonAsHighlighted ? juce::Colour(0xff4a463d) : juce::Colour(0xff39362f);
    juce::Colour bottom = juce::Colour(0xff1a1915);
    if (shouldDrawButtonAsDown)
        std::swap(top, bottom);

    g.setColour(juce::Colours::black.withAlpha(0.55f));
    g.fillRoundedRectangle(bounds.translated(0.0f, 1.5f), 5.0f);

    juce::ColourGradient grad(top, bounds.getX(), bounds.getY(), bottom, bounds.getX(), bounds.getBottom(), false);
    g.setGradientFill(grad);
    g.fillRoundedRectangle(bounds, 5.0f);

    g.setColour(lit ? amberGlow.withAlpha(0.9f) : juce::Colour(0xff0a0907));
    g.drawRoundedRectangle(bounds, 5.0f, lit ? 1.6f : 1.3f);
    g.setColour(juce::Colour(0xff6e6858).withAlpha(0.35f));
    g.drawRoundedRectangle(bounds.reduced(1.5f), 4.0f, 1.0f);
}

void SlateLookAndFeel::drawButtonText(juce::Graphics& g, juce::TextButton& button,
                                      bool /*shouldDrawButtonAsHighlighted*/,
                                      bool /*shouldDrawButtonAsDown*/)
{
    const bool lit = button.getToggleState();
    g.setFont(boldFont(juce::jlimit(11.0f, 20.0f, static_cast<float>(button.getHeight()) * 0.45f)));
    g.setColour(lit ? amberGlow : textLight);
    g.drawText(button.getButtonText(), button.getLocalBounds().reduced(4, 0), juce::Justification::centred, true);
}

void SlateLookAndFeel::drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
                                        bool /*shouldDrawButtonAsHighlighted*/,
                                        bool /*shouldDrawButtonAsDown*/)
{
    const bool on = button.getToggleState();
    const float s = juce::jmin(18.0f, static_cast<float>(button.getHeight()) - 6.0f);
    juce::Rectangle<float> box(4.0f, (static_cast<float>(button.getHeight()) - s) * 0.5f, s, s);

    g.setColour(juce::Colour(0xff0e0d0b));
    g.fillRoundedRectangle(box, 3.0f);
    g.setColour(slateBorder);
    g.drawRoundedRectangle(box, 3.0f, 1.2f);
    if (on)
    {
        g.setColour(amberGlow);
        g.fillRoundedRectangle(box.reduced(3.5f), 2.0f);
    }

    g.setColour(textLight);
    g.setFont(boldFont(13.0f));
    g.drawText(button.getButtonText(), button.getLocalBounds().withTrimmedLeft(static_cast<int>(s) + 12),
               juce::Justification::centredLeft, true);
}

void SlateLookAndFeel::drawComboBox(juce::Graphics& g, int width, int height, bool /*isButtonDown*/,
                                    int /*buttonX*/, int /*buttonY*/, int /*buttonW*/, int /*buttonH*/,
                                    juce::ComboBox& box)
{
    juce::Rectangle<float> r(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height));

    juce::ColourGradient grad(juce::Colour(0xff161512), 0.0f, 0.0f, juce::Colour(0xff2a2822), 0.0f, r.getBottom(), false);
    g.setGradientFill(grad);
    g.fillRoundedRectangle(r.reduced(1.0f), 5.0f);
    g.setColour(juce::Colour(0xff000000).withAlpha(0.7f));
    g.drawRoundedRectangle(r.reduced(1.0f), 5.0f, 1.6f);
    g.setColour(juce::Colour(0xff5a5446).withAlpha(0.5f));
    g.drawRoundedRectangle(r.reduced(3.0f), 4.0f, 1.0f);

    if (! box.getProperties().getWithDefault("noArrow", false))
    {
        juce::Path arrow;
        const float ax = r.getRight() - 16.0f, ay = r.getCentreY();
        arrow.addTriangle(ax - 5.0f, ay - 3.0f, ax + 5.0f, ay - 3.0f, ax, ay + 4.0f);
        g.setColour(textLight);
        g.fillPath(arrow);
    }
}

void SlateLookAndFeel::positionComboBoxText(juce::ComboBox& box, juce::Label& label)
{
    const int inset = static_cast<int>(box.getProperties().getWithDefault("textInset", 10));
    label.setBounds(inset, 1, box.getWidth() - inset * 2, box.getHeight() - 2);
    label.setFont(boldFont(static_cast<float>(box.getHeight()) * 0.50f));
}

void SlateLookAndFeel::drawPopupMenuBackground(juce::Graphics& g, int width, int height)
{
    g.fillAll(juce::Colour(0xff1a1916));
    g.setColour(slateHighlight);
    g.drawRect(0, 0, width, height, 1);
}

void SlateLookAndFeel::drawPopupMenuItem(juce::Graphics& g, const juce::Rectangle<int>& area,
                                         bool isSeparator, bool isActive, bool isHighlighted,
                                         bool isTicked, bool hasSubMenu,
                                         const juce::String& text, const juce::String& /*shortcutKeyText*/,
                                         const juce::Drawable* /*icon*/, const juce::Colour* textColour)
{
    if (isSeparator)
    {
        g.setColour(slateBorder);
        g.fillRect(area.reduced(8, 0).withHeight(1).withY(area.getCentreY()));
        return;
    }

    auto r = area.reduced(2, 1);
    if (isHighlighted && isActive)
    {
        g.setColour(amberGlow.withAlpha(0.28f));
        g.fillRoundedRectangle(r.toFloat(), 3.0f);
    }

    g.setColour(!isActive ? textDim.withAlpha(0.5f)
                          : (textColour != nullptr ? *textColour : (isHighlighted ? amberGlow : textLight)));
    g.setFont(boldFont(13.0f));
    g.drawText(text, r.withTrimmedLeft(26), juce::Justification::centredLeft, true);

    if (isTicked)
    {
        g.setColour(amberGlow);
        g.fillEllipse(static_cast<float>(r.getX()) + 9.0f, static_cast<float>(r.getCentreY()) - 3.0f, 6.0f, 6.0f);
    }

    if (hasSubMenu)
    {
        g.setColour(textLight);
        g.drawText(">", r.withTrimmedRight(8), juce::Justification::centredRight);
    }
}

} // namespace yeahwaah
