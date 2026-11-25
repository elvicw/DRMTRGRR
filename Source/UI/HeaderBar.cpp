#include "HeaderBar.h"
#include "SlateLookAndFeel.h"
#include "../PluginProcessor.h"

namespace yeahwaah
{

namespace
{
constexpr int kKnobCentres[3] = { 1100, 1264, 1425 };
constexpr int kKnobWidth = 100;

float peakToMeter(float peak)
{
    return juce::jlimit(0.0f, 1.0f, (juce::Decibels::gainToDecibels(peak + 1.0e-5f) + 48.0f) / 48.0f);
}
} // namespace

HeaderBar::HeaderBar(DrmtrgrrAudioProcessor& proc)
    : processor(proc),
      inputKnob(proc.getAPVTS(), "inputGain", "INPUT", " dB", 1),
      mixKnob(proc.getAPVTS(), "mix", "MIX", "%", 0),
      outputKnob(proc.getAPVTS(), "outputGain", "OUTPUT", " dB", 1)
{
    browserTabButton.onClick = [this] {
        setActiveTab(currentTab == ActiveTab::Browser ? ActiveTab::Trigger : ActiveTab::Browser);
    };
    browserTabButton.setTooltip("Sample browser");
    addAndMakeVisible(browserTabButton);

    settingsTabButton.onClick = [this] {
        setActiveTab(currentTab == ActiveTab::Settings ? ActiveTab::Trigger : ActiveTab::Settings);
    };
    settingsTabButton.setTooltip("Settings");
    addAndMakeVisible(settingsTabButton);
    updateTabButtons();

    for (int i = 0; i < processor.getNumPrograms(); ++i)
        presetBox.addItem(processor.getProgramName(i), i + 1);

    presetBox.getProperties().set("noArrow", true);
    presetBox.getProperties().set("textInset", 44);
    presetBox.setJustificationType(juce::Justification::centredLeft);
    presetBox.setSelectedId(processor.getCurrentProgram() + 1, juce::dontSendNotification);
    presetBox.onChange = [this] {
        int id = presetBox.getSelectedId() - 1;
        if (id >= 0)
            processor.setCurrentProgram(id);
    };
    presetBox.setTooltip("Preset");
    addAndMakeVisible(presetBox);

    prevPresetBtn.onClick = [this] {
        int id = presetBox.getSelectedId() - 1;
        if (id > 0)
            presetBox.setSelectedId(id);
    };
    prevPresetBtn.setTooltip("Previous preset");
    addAndMakeVisible(prevPresetBtn);

    nextPresetBtn.onClick = [this] {
        int id = presetBox.getSelectedId() + 1;
        if (id <= processor.getNumPrograms())
            presetBox.setSelectedId(id);
    };
    nextPresetBtn.setTooltip("Next preset");
    addAndMakeVisible(nextPresetBtn);

    addAndMakeVisible(inputKnob);
    addAndMakeVisible(mixKnob);
    addAndMakeVisible(outputKnob);
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
    settingsTabButton.setToggleState(currentTab == ActiveTab::Settings, juce::dontSendNotification);
}

void HeaderBar::updateMeters(float inPeak, float outPeak)
{
    const float in = peakToMeter(inPeak);
    const float out = peakToMeter(outPeak);
    if (std::abs(in - currentInPeak) < 0.01f && std::abs(out - currentOutPeak) < 0.01f)
        return;

    currentInPeak = in;
    currentOutPeak = out;
    repaint();
}

void HeaderBar::paint(juce::Graphics& g)
{
    // Waveform logo mark
    juce::Path wave;
    const float baseY = 70.0f;
    wave.startNewSubPath(68.0f, baseY);
    wave.lineTo(88.0f, baseY);
    const float pts[][2] = { { 94, -4 }, { 99, 9 }, { 105, -26 }, { 110, 28 }, { 116, -18 }, { 121, 14 }, { 126, -6 }, { 131, 0 }, { 143, 0 } };
    float x = 88.0f;
    for (auto& p : pts)
    {
        x = 88.0f + (p[0] - 88.0f);
        wave.lineTo(x, baseY + p[1]);
    }
    g.setColour(SlateLookAndFeel::titleAmber.withAlpha(0.35f));
    g.strokePath(wave, juce::PathStrokeType(4.0f));
    g.setColour(SlateLookAndFeel::titleAmber);
    g.strokePath(wave, juce::PathStrokeType(1.8f));

    hw::drawEngravedText(g, "YEAHWAAH", { 154, 50, 160, 30 }, 29.0f, SlateLookAndFeel::textLight, juce::Justification::centredLeft);
    hw::drawEngravedText(g, "AUDIO", { 156, 78, 120, 18 }, 15.0f, SlateLookAndFeel::titleAmber, juce::Justification::centredLeft);

    // Product title in brushed-steel lettering
    auto titleArea = juce::Rectangle<int>(322, 26, 350, 78);
    g.setFont(SlateLookAndFeel::boldFont(60.0f));
    g.setColour(juce::Colours::black.withAlpha(0.85f));
    g.drawText("DRM-TRGRR", titleArea.translated(3, 4), juce::Justification::centredLeft, true);
    juce::ColourGradient steel(juce::Colour(0xffd9d4c6), 0.0f, 34.0f, juce::Colour(0xff77726a), 0.0f, 92.0f, false);
    g.setGradientFill(steel);
    g.drawText("DRM-TRGRR", titleArea, juce::Justification::centredLeft, true);

    // Separators between the master knobs
    g.setColour(juce::Colours::black.withAlpha(0.7f));
    for (int sx : { 1022, 1182, 1348 })
        g.fillRect(sx, 14, 2, 106);
    g.setColour(juce::Colour(0xff6e6858).withAlpha(0.35f));
    for (int sx : { 1022, 1182, 1348 })
        g.fillRect(sx + 2, 14, 1, 106);

    // Input (left) and output (right) LED meters
    auto drawMeter = [&](float mx, float level) {
        g.setColour(juce::Colour(0xff0a0907));
        g.fillRoundedRectangle(mx - 4.0f, 52.0f, 30.0f, 72.0f, 3.0f);
        hw::drawLedBar(g, { mx, 56.0f, 22.0f, 64.0f }, level, 7);
    };
    drawMeter(24.0f, currentInPeak);
    drawMeter(1490.0f, currentOutPeak);
}

void HeaderBar::resized()
{
    for (int i = 0; i < 3; ++i)
    {
        juce::Rectangle<int> r(kKnobCentres[i] - kKnobWidth / 2, 12, kKnobWidth, 114);
        (i == 0 ? inputKnob : i == 1 ? mixKnob : outputKnob).setBounds(r);
    }

    presetBox.setBounds(682, 38, 313, 44);
    prevPresetBtn.setBounds(690, 48, 24, 24);
    nextPresetBtn.setBounds(956, 48, 24, 24);

    browserTabButton.setBounds(682, 92, 152, 26);
    settingsTabButton.setBounds(843, 92, 152, 26);
}

} // namespace yeahwaah
