#include "HeaderBar.h"
#include "SlateLookAndFeel.h"
#include "../PluginProcessor.h"

namespace yeahwaah
{

HeaderBar::HeaderBar(DrmtrgrrAudioProcessor& proc)
    : processor(proc)
{
    // 1. Slate Navigation Tabs
    browserTabButton.setClickingTogglesState(false);
    browserTabButton.onClick = [this] { setActiveTab(ActiveTab::Browser); };
    addAndMakeVisible(browserTabButton);

    triggerTabButton.setClickingTogglesState(false);
    triggerTabButton.onClick = [this] { setActiveTab(ActiveTab::Trigger); };
    addAndMakeVisible(triggerTabButton);

    settingsTabButton.setClickingTogglesState(false);
    settingsTabButton.onClick = [this] { setActiveTab(ActiveTab::Settings); };
    addAndMakeVisible(settingsTabButton);

    updateTabButtons();

    // 2. Preset Selection
    prevPresetBtn.onClick = [this] {
        int id = presetBox.getSelectedId() - 1;
        if (id > 0)
            presetBox.setSelectedId(id);
    };
    addAndMakeVisible(prevPresetBtn);

    for (int i = 0; i < processor.getNumPrograms(); ++i)
        presetBox.addItem(processor.getProgramName(i), i + 1);

    presetBox.setSelectedId(processor.getCurrentProgram() + 1, juce::dontSendNotification);
    presetBox.onChange = [this] {
        int id = presetBox.getSelectedId() - 1;
        if (id >= 0)
            processor.setCurrentProgram(id);
    };
    addAndMakeVisible(presetBox);

    nextPresetBtn.onClick = [this] {
        int id = presetBox.getSelectedId() + 1;
        if (id <= processor.getNumPrograms())
            presetBox.setSelectedId(id);
    };
    addAndMakeVisible(nextPresetBtn);

    // 3. Input Trim Knob
    inputGainSlider.setName("Input");
    inputGainSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    inputGainSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 46, 14);
    inputGainSlider.setTextValueSuffix(" dB");
    addAndMakeVisible(inputGainSlider);
    inGainAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processor.getAPVTS(), "inputGain", inputGainSlider);

    inLabel.setText("INPUT", juce::dontSendNotification);
    inLabel.setFont(juce::Font(9.0f, juce::Font::bold));
    inLabel.setColour(juce::Label::textColourId, SlateLookAndFeel::textDim);
    inLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(inLabel);

    // 4. Dry / Wet Mix Knob
    mixSlider.setName("Mix");
    mixSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    mixSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 46, 14);
    mixSlider.setTextValueSuffix("%");
    addAndMakeVisible(mixSlider);
    mixAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processor.getAPVTS(), "mix", mixSlider);

    mixLabel.setText("MIX", juce::dontSendNotification);
    mixLabel.setFont(juce::Font(9.0f, juce::Font::bold));
    mixLabel.setColour(juce::Label::textColourId, SlateLookAndFeel::textDim);
    mixLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(mixLabel);

    // 5. Master Output Level Knob
    outputGainSlider.setName("Output");
    outputGainSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    outputGainSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 46, 14);
    outputGainSlider.setTextValueSuffix(" dB");
    addAndMakeVisible(outputGainSlider);
    outGainAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processor.getAPVTS(), "outputGain", outputGainSlider);

    outLabel.setText("OUTPUT", juce::dontSendNotification);
    outLabel.setFont(juce::Font(9.0f, juce::Font::bold));
    outLabel.setColour(juce::Label::textColourId, SlateLookAndFeel::textDim);
    outLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(outLabel);
}

void HeaderBar::setActiveTab(ActiveTab tab)
{
    currentTab = tab;
    updateTabButtons();
    if (onTabChanged)
        onTabChanged(tab);
    repaint();
}

void HeaderBar::updateTabButtons()
{
    browserTabButton.setToggleState(currentTab == ActiveTab::Browser, juce::dontSendNotification);
    triggerTabButton.setToggleState(currentTab == ActiveTab::Trigger, juce::dontSendNotification);
    settingsTabButton.setToggleState(currentTab == ActiveTab::Settings, juce::dontSendNotification);
}

void HeaderBar::updateMeters(float inPeak, float outPeak)
{
    currentInPeak = inPeak;
    currentOutPeak = outPeak;
    repaint();
}

void HeaderBar::paint(juce::Graphics& g)
{
    const int w = getWidth();
    const int h = getHeight();

    // Brushed dark charcoal metallic chassis
    juce::ColourGradient bgGrad(juce::Colour(0xff222733), 0.0f, 0.0f,
                                juce::Colour(0xff11141a), 0.0f, static_cast<float>(h), false);
    g.setGradientFill(bgGrad);
    g.fillRect(0, 0, w, h);

    // Top subtle highlight line
    g.setColour(juce::Colour(0xff3f4a5c));
    g.drawHorizontalLine(0, 0.0f, static_cast<float>(w));

    // Bottom hairline metallic divider groove
    g.setColour(SlateLookAndFeel::slateBorder.darker(0.3f));
    g.drawHorizontalLine(h - 2, 0.0f, static_cast<float>(w));
    g.setColour(SlateLookAndFeel::slateHighlight);
    g.drawHorizontalLine(h - 1, 0.0f, static_cast<float>(w));

    // Analog rack chassis screws in top corners
    auto drawScrew = [&](float cx, float cy) {
        g.setColour(juce::Colour(0xff06070a));
        g.fillEllipse(cx - 6.0f, cy - 6.0f, 12.0f, 12.0f);

        juce::ColourGradient screwGrad(juce::Colour(0xff4c5767), cx - 2.5f, cy - 2.5f,
                                       juce::Colour(0xff16191f), cx + 3.5f, cy + 3.5f, true);
        g.setGradientFill(screwGrad);
        g.fillEllipse(cx - 5.0f, cy - 5.0f, 10.0f, 10.0f);

        g.setColour(juce::Colour(0xff0a0c10));
        g.drawLine(cx - 3.5f, cy, cx + 3.5f, cy, 1.4f);
        g.drawLine(cx, cy - 3.5f, cx, cy + 3.5f, 1.4f);
    };
    drawScrew(9.0f, 9.0f);
    drawScrew(static_cast<float>(w) - 9.0f, 9.0f);

    // Slate Logo Badge Area
    // Company Sub-title: "YEAHWAAH AUDIO"
    g.setFont(juce::Font(9.5f, juce::Font::bold));
    g.setColour(SlateLookAndFeel::amberGlow);
    g.drawText("YEAHWAAH", 24, 7, 120, 12, juce::Justification::left);

    // Main Logo: "DRMTRGRR" in bold platinum metallic typography
    g.setFont(juce::Font(21.0f, juce::Font::bold));
    g.setColour(juce::Colour(0xff040507));
    g.drawText("DRMTRGRR", 25, 20, 150, 24, juce::Justification::left);
    g.setColour(SlateLookAndFeel::textLight);
    g.drawText("DRMTRGRR", 24, 19, 150, 24, juce::Justification::left);

    // Badge: "TRIGGER 2 PLATINUM"
    juce::Rectangle<float> badgeRect(24.0f, 43.0f, 128.0f, 13.0f);
    g.setColour(juce::Colour(0xff121620));
    g.fillRoundedRectangle(badgeRect, 2.0f);
    g.setColour(SlateLookAndFeel::cyanGlow.withAlpha(0.7f));
    g.drawRoundedRectangle(badgeRect, 2.0f, 1.0f);
    g.setFont(juce::Font(8.0f, juce::Font::bold));
    g.setColour(SlateLookAndFeel::cyanGlow);
    g.drawText("TRIGGER 2 PLATINUM", badgeRect.toNearestInt(), juce::Justification::centred);

    // Active Tab Glowing Underline Bar
    juce::Rectangle<int> activeTabBounds;
    if (currentTab == ActiveTab::Browser) activeTabBounds = browserTabButton.getBounds();
    else if (currentTab == ActiveTab::Trigger) activeTabBounds = triggerTabButton.getBounds();
    else if (currentTab == ActiveTab::Settings) activeTabBounds = settingsTabButton.getBounds();

    if (!activeTabBounds.isEmpty())
    {
        float barX = static_cast<float>(activeTabBounds.getX());
        float barW = static_cast<float>(activeTabBounds.getWidth());
        float barY = static_cast<float>(activeTabBounds.getBottom()) + 2.0f;

        // Glow
        g.setColour(SlateLookAndFeel::cyanGlow.withAlpha(0.4f));
        g.fillRoundedRectangle(barX - 2.0f, barY - 1.0f, barW + 4.0f, 4.0f, 2.0f);
        // Core
        g.setColour(SlateLookAndFeel::cyanGlow);
        g.fillRoundedRectangle(barX, barY, barW, 2.0f, 1.0f);
    }

    // 8-Segment High-Resolution Input Meter
    const float inMeterX = static_cast<float>(w) - 245.0f;
    const float meterY = 12.0f;
    const float meterW = 7.0f;
    const int numLeds = 8;
    const float ledH = 4.2f;
    const float ledGap = 1.3f;

    g.setFont(juce::Font(8.5f, juce::Font::bold));
    g.setColour(SlateLookAndFeel::textDim);
    g.drawText("IN", static_cast<int>(inMeterX) - 18, static_cast<int>(meterY) + 14, 16, 12, juce::Justification::centredRight);

    int inLedsLit = static_cast<int>(juce::jlimit(0.0f, 1.0f, currentInPeak) * static_cast<float>(numLeds));
    for (int l = 0; l < numLeds; ++l)
    {
        float ly = meterY + static_cast<float>(numLeds - 1 - l) * (ledH + ledGap);
        bool isLit = (l < inLedsLit);
        juce::Colour c = (l >= 7) ? SlateLookAndFeel::muteRed : (l >= 5) ? SlateLookAndFeel::amberGlow : SlateLookAndFeel::cyanGlow;
        if (isLit)
        {
            g.setColour(c.withAlpha(0.4f));
            g.fillRoundedRectangle(inMeterX - 1.0f, ly - 1.0f, meterW + 2.0f, ledH + 2.0f, 1.5f);
            g.setColour(c);
            g.fillRoundedRectangle(inMeterX, ly, meterW, ledH, 1.0f);
        }
        else
        {
            g.setColour(c.withAlpha(0.12f));
            g.fillRoundedRectangle(inMeterX, ly, meterW, ledH, 1.0f);
        }
    }

    // 8-Segment High-Resolution Output Meter
    const float outMeterX = static_cast<float>(w) - 22.0f;
    g.drawText("OUT", static_cast<int>(outMeterX) - 22, static_cast<int>(meterY) + 14, 20, 12, juce::Justification::centredRight);

    int outLedsLit = static_cast<int>(juce::jlimit(0.0f, 1.0f, currentOutPeak) * static_cast<float>(numLeds));
    for (int l = 0; l < numLeds; ++l)
    {
        float ly = meterY + static_cast<float>(numLeds - 1 - l) * (ledH + ledGap);
        bool isLit = (l < outLedsLit);
        juce::Colour c = (l >= 7) ? SlateLookAndFeel::muteRed : (l >= 5) ? SlateLookAndFeel::amberGlow : SlateLookAndFeel::cyanGlow;
        if (isLit)
        {
            g.setColour(c.withAlpha(0.4f));
            g.fillRoundedRectangle(outMeterX - 1.0f, ly - 1.0f, meterW + 2.0f, ledH + 2.0f, 1.5f);
            g.setColour(c);
            g.fillRoundedRectangle(outMeterX, ly, meterW, ledH, 1.0f);
        }
        else
        {
            g.setColour(c.withAlpha(0.12f));
            g.fillRoundedRectangle(outMeterX, ly, meterW, ledH, 1.0f);
        }
    }
}

void HeaderBar::resized()
{
    const int w = getWidth();

    // 1. Navigation Tabs (Slate Trigger 2 Style)
    const int tabY = 16;
    const int tabH = 28;
    const int tabW = 95;
    int tabX = 175;

    browserTabButton.setBounds(tabX, tabY, tabW, tabH);
    tabX += tabW + 6;
    triggerTabButton.setBounds(tabX, tabY, tabW, tabH);
    tabX += tabW + 6;
    settingsTabButton.setBounds(tabX, tabY, tabW, tabH);

    // 2. Preset Selection Bar
    const int presetX = tabX + tabW + 18;
    prevPresetBtn.setBounds(presetX, tabY, 24, tabH);
    presetBox.setBounds(presetX + 26, tabY, 150, tabH);
    nextPresetBtn.setBounds(presetX + 178, tabY, 24, tabH);

    // 3. Master Controls on Right
    inputGainSlider.setBounds(w - 232, 11, 46, 46);
    inLabel.setBounds(w - 232, 1, 46, 10);

    mixSlider.setBounds(w - 156, 11, 46, 46);
    mixLabel.setBounds(w - 156, 1, 46, 10);

    outputGainSlider.setBounds(w - 84, 11, 46, 46);
    outLabel.setBounds(w - 84, 1, 46, 10);
}

} // namespace yeahwaah
