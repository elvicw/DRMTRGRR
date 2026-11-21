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
    inputGainSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    inputGainSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 44, 14);
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
    mixSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    mixSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 44, 14);
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
    outputGainSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    outputGainSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 44, 14);
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

    // Brushed dark charcoal chassis
    juce::ColourGradient bgGrad(juce::Colour(0xff222630), 0.0f, 0.0f,
                                juce::Colour(0xff13151a), 0.0f, (float)h, false);
    g.setGradientFill(bgGrad);
    g.fillRect(0, 0, w, h);

    // Bottom hairline metallic divider
    g.setColour(SlateLookAndFeel::slateBorder);
    g.drawHorizontalLine(h - 1, 0.0f, (float)w);

    // Analog rack chassis screws in top corners
    auto drawScrew = [&](float cx, float cy) {
        g.setColour(juce::Colour(0xff080a0c));
        g.fillEllipse(cx - 5.5f, cy - 5.5f, 11.0f, 11.0f);
        juce::ColourGradient screwGrad(juce::Colour(0xff4a5360), cx - 2.0f, cy - 2.0f,
                                       juce::Colour(0xff181b20), cx + 3.0f, cy + 3.0f, true);
        g.setGradientFill(screwGrad);
        g.fillEllipse(cx - 4.5f, cy - 4.5f, 9.0f, 9.0f);
        g.setColour(juce::Colour(0xff090b0e));
        g.drawLine(cx - 3.0f, cy, cx + 3.0f, cy, 1.2f);
        g.drawLine(cx, cy - 3.0f, cx, cy + 3.0f, 1.2f);
    };
    drawScrew(8.0f, 8.0f);
    drawScrew((float)w - 8.0f, 8.0f);

    // Company Logo ("yeahwaah")
    g.setFont(juce::Font(10.5f, juce::Font::bold));
    g.setColour(SlateLookAndFeel::cyanGlow);
    g.drawText("YEAHWAAH", 24, 6, 110, 14, juce::Justification::left);

    // Plugin Name ("DRMTRGRR")
    g.setFont(juce::Font(22.0f, juce::Font::bold));
    g.setColour(juce::Colour(0xff050709));
    g.drawText("DRMTRGRR", 25, 20, 150, 24, juce::Justification::left);
    g.setColour(SlateLookAndFeel::textLight);
    g.drawText("DRMTRGRR", 24, 19, 150, 24, juce::Justification::left);

    // Glowing cyan underline
    g.setColour(SlateLookAndFeel::cyanGlow.withAlpha(0.75f));
    g.fillRect(24.0f, 43.0f, 116.0f, 2.0f);

    // 6-Segment Input Meter
    const float inMeterX = (float)w - 235.0f;
    const float meterY = 12.0f;
    const float meterW = 6.5f;
    const int numLeds = 6;
    const float ledH = 4.5f;

    g.setFont(juce::Font(8.0f, juce::Font::bold));
    g.setColour(SlateLookAndFeel::textDim);
    g.drawText("IN", (int)inMeterX - 16, (int)meterY + 8, 14, 10, juce::Justification::centredRight);

    int inLedsLit = static_cast<int>(juce::jlimit(0.0f, 1.0f, currentInPeak) * numLeds);
    for (int l = 0; l < numLeds; ++l)
    {
        float ly = meterY + (numLeds - 1 - l) * (ledH + 1.2f);
        bool isLit = (l < inLedsLit);
        juce::Colour c = (l >= 5) ? SlateLookAndFeel::muteRed : (l >= 4) ? SlateLookAndFeel::amberGlow : SlateLookAndFeel::cyanGlow;
        g.setColour(isLit ? c : c.withAlpha(0.12f));
        g.fillRoundedRectangle(inMeterX, ly, meterW, ledH, 1.0f);
    }

    // 6-Segment Output Meter
    const float outMeterX = (float)w - 22.0f;
    g.drawText("OUT", (int)outMeterX - 18, (int)meterY + 8, 16, 10, juce::Justification::centredRight);

    int outLedsLit = static_cast<int>(juce::jlimit(0.0f, 1.0f, currentOutPeak) * numLeds);
    for (int l = 0; l < numLeds; ++l)
    {
        float ly = meterY + (numLeds - 1 - l) * (ledH + 1.2f);
        bool isLit = (l < outLedsLit);
        juce::Colour c = (l >= 5) ? SlateLookAndFeel::muteRed : (l >= 4) ? SlateLookAndFeel::amberGlow : SlateLookAndFeel::cyanGlow;
        g.setColour(isLit ? c : c.withAlpha(0.12f));
        g.fillRoundedRectangle(outMeterX, ly, meterW, ledH, 1.0f);
    }
}

void HeaderBar::resized()
{
    const int w = getWidth();

    // 1. Navigation Tabs (Slate Trigger 2 Style)
    const int tabY = 16;
    const int tabH = 26;
    const int tabW = 86;
    int tabX = 165;

    browserTabButton.setBounds(tabX, tabY, tabW, tabH);
    tabX += tabW + 6;
    triggerTabButton.setBounds(tabX, tabY, tabW, tabH);
    tabX += tabW + 6;
    settingsTabButton.setBounds(tabX, tabY, tabW, tabH);

    // 2. Preset Selection
    const int presetX = tabX + tabW + 16;
    prevPresetBtn.setBounds(presetX, tabY, 24, tabH);
    presetBox.setBounds(presetX + 26, tabY, 150, tabH);
    nextPresetBtn.setBounds(presetX + 178, tabY, 24, tabH);

    // 3. Master Controls on Right
    inputGainSlider.setBounds(w - 225, 12, 44, 46);
    inLabel.setBounds(w - 225, 2, 44, 10);

    mixSlider.setBounds(w - 150, 12, 44, 46);
    mixLabel.setBounds(w - 150, 2, 44, 10);

    outputGainSlider.setBounds(w - 82, 12, 44, 46);
    outLabel.setBounds(w - 82, 2, 44, 10);
}

} // namespace yeahwaah
