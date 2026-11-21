#include "DetectorPanel.h"
#include "SlateLookAndFeel.h"

namespace yeahwaah
{

DetectorPanel::DetectorPanel(juce::AudioProcessorValueTreeState& vts)
    : apvts(vts)
{
    // 1. Detection Knobs
    setupKnob(inputGainSlider, inputGainLabel, "INPUT TRIM", " dB");
    inputGainAtt = std::make_unique<SliderAttachment>(apvts, "inputGain", inputGainSlider);

    setupKnob(detailSlider, detailLabel, "DETAIL", " dB");
    detailAtt = std::make_unique<SliderAttachment>(apvts, "detail", detailSlider);

    setupKnob(sensitivitySlider, sensLabel, "SENSITIVITY", "%");
    sensAtt = std::make_unique<SliderAttachment>(apvts, "sensitivity", sensitivitySlider);

    setupKnob(retriggerSlider, retriggerLabel, "RETRIGGER", " ms");
    retriggerAtt = std::make_unique<SliderAttachment>(apvts, "retrigger", retriggerSlider);

    setupKnob(dynamicsSlider, dynLabel, "DYNAMICS", "%");
    dynAtt = std::make_unique<SliderAttachment>(apvts, "dynamics", dynamicsSlider);

    // 2. Filter Knobs
    setupKnob(lowCutSlider, lowCutLabel, "LOW-CUT", " Hz");
    lowCutAtt = std::make_unique<SliderAttachment>(apvts, "lowCut", lowCutSlider);

    setupKnob(highCutSlider, highCutLabel, "HIGH-CUT", " Hz");
    highCutAtt = std::make_unique<SliderAttachment>(apvts, "highCut", highCutSlider);

    auditionButton.setClickingTogglesState(true);
    addAndMakeVisible(auditionButton);
    auditionAtt = std::make_unique<ButtonAttachment>(apvts, "auditionFilter", auditionButton);

    // 3. Gate
    gateEnableButton.setClickingTogglesState(true);
    addAndMakeVisible(gateEnableButton);
    gateEnableAtt = std::make_unique<ButtonAttachment>(apvts, "gateEnable", gateEnableButton);

    setupKnob(gateThresholdSlider, gateThreshLabel, "THRESHOLD", " dB");
    gateThreshAtt = std::make_unique<SliderAttachment>(apvts, "gateThreshold", gateThresholdSlider);

    setupKnob(gateAttackSlider, gateAttLabel, "ATTACK", " ms");
    gateAttAtt = std::make_unique<SliderAttachment>(apvts, "gateAttack", gateAttackSlider);

    setupKnob(gateHoldSlider, gateHldLabel, "HOLD", " ms");
    gateHldAtt = std::make_unique<SliderAttachment>(apvts, "gateHold", gateHoldSlider);

    setupKnob(gateReleaseSlider, gateRelLabel, "RELEASE", " ms");
    gateRelAtt = std::make_unique<SliderAttachment>(apvts, "gateRelease", gateReleaseSlider);

    // 4. Leakage & MIDI
    leakageEnableButton.setClickingTogglesState(true);
    addAndMakeVisible(leakageEnableButton);
    leakageEnableAtt = std::make_unique<ButtonAttachment>(apvts, "leakageEnable", leakageEnableButton);

    setupKnob(leakageAmountSlider, leakLabel, "AMOUNT", "%");
    leakageAmountAtt = std::make_unique<SliderAttachment>(apvts, "leakageAmount", leakageAmountSlider);

    midiOutButton.setClickingTogglesState(true);
    addAndMakeVisible(midiOutButton);
    midiOutAtt = std::make_unique<ButtonAttachment>(apvts, "midiOutEnable", midiOutButton);

    setupKnob(midiNoteSlider, midiNoteLabel, "NOTE", "");
    midiNoteSlider.textFromValueFunction = [](double v) {
        int note = static_cast<int>(v);
        return juce::MidiMessage::getMidiNoteName(note, true, true, 3);
    };
    midiNoteAtt = std::make_unique<SliderAttachment>(apvts, "midiNote", midiNoteSlider);

    setupKnob(midiChannelSlider, midiChLabel, "CHANNEL", "");
    midiChannelAtt = std::make_unique<SliderAttachment>(apvts, "midiChannel", midiChannelSlider);
}

void DetectorPanel::setupKnob(juce::Slider& s, juce::Label& l, const juce::String& text, const juce::String& suffix)
{
    s.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    s.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 55, 16);
    s.setTextValueSuffix(suffix);
    addAndMakeVisible(s);

    l.setText(text, juce::dontSendNotification);
    l.setFont(juce::Font(9.5f, juce::Font::bold));
    l.setJustificationType(juce::Justification::centred);
    l.setColour(juce::Label::textColourId, SlateLookAndFeel::textDim);
    addAndMakeVisible(l);
}

void DetectorPanel::paint(juce::Graphics& g)
{
    const int w = getWidth();
    const int h = getHeight();

    // Dark slate panel fill
    g.setColour(SlateLookAndFeel::bgPanel);
    g.fillRect(0, 0, w, h);

    // Draw section divider lines & section title headers
    g.setFont(juce::Font(10.5f, juce::Font::bold));
    g.setColour(SlateLookAndFeel::textLight.withAlpha(0.85f));

    // Section 1: Transient Engine
    g.drawText("TRANSIENT ENGINE", 12, 4, 220, 16, juce::Justification::left);

    // Section 2: Filter Section
    g.drawText("BANDPASS FILTER", 355, 4, 160, 16, juce::Justification::left);

    // Section 3: Gate
    g.drawText("TRANSIENT GATE", 545, 4, 180, 16, juce::Justification::left);

    // Section 4: Bleed & MIDI
    g.drawText("BLEED & MIDI", 815, 4, 160, 16, juce::Justification::left);

    // Vertical dividers
    g.setColour(SlateLookAndFeel::slateBorder);
    g.drawVerticalLine(340, 4.0f, (float)h - 4.0f);
    g.drawVerticalLine(530, 4.0f, (float)h - 4.0f);
    g.drawVerticalLine(800, 4.0f, (float)h - 4.0f);

    // Outer border
    g.drawRect(0, 0, w, h, 1);
}

void DetectorPanel::resized()
{
    const int knobY = 22;
    const int knobW = 60;
    const int knobH = 68;

    // 1. Transient Engine Knobs: Input, Detail, Sens, Retrig, Dyn
    int x = 8;
    inputGainLabel.setBounds(x, knobY, knobW, 14);
    inputGainSlider.setBounds(x, knobY + 12, knobW, knobH);

    x += 66;
    detailLabel.setBounds(x, knobY, knobW, 14);
    detailSlider.setBounds(x, knobY + 12, knobW, knobH);

    x += 66;
    sensLabel.setBounds(x, knobY, knobW, 14);
    sensitivitySlider.setBounds(x, knobY + 12, knobW, knobH);

    x += 66;
    retriggerLabel.setBounds(x, knobY, knobW, 14);
    retriggerSlider.setBounds(x, knobY + 12, knobW, knobH);

    x += 66;
    dynLabel.setBounds(x, knobY, knobW, 14);
    dynamicsSlider.setBounds(x, knobY + 12, knobW, knobH);

    // 2. Filter: Low-Cut, High-Cut, Listen button
    x = 350;
    lowCutLabel.setBounds(x, knobY, knobW, 14);
    lowCutSlider.setBounds(x, knobY + 12, knobW, knobH);

    x += 64;
    highCutLabel.setBounds(x, knobY, knobW, 14);
    highCutSlider.setBounds(x, knobY + 12, knobW, knobH);

    auditionButton.setBounds(x + 60, knobY + 24, 52, 28);

    // 3. Gate: Enable, Thresh, Attack, Hold, Rel
    x = 540;
    gateEnableButton.setBounds(x, knobY + 24, 60, 24);

    x += 64;
    gateThreshLabel.setBounds(x, knobY, 48, 14);
    gateThresholdSlider.setBounds(x, knobY + 12, 48, knobH);

    x += 50;
    gateAttLabel.setBounds(x, knobY, 46, 14);
    gateAttackSlider.setBounds(x, knobY + 12, 46, knobH);

    x += 48;
    gateHldLabel.setBounds(x, knobY, 46, 14);
    gateHoldSlider.setBounds(x, knobY + 12, 46, knobH);

    x += 48;
    gateRelLabel.setBounds(x, knobY, 46, 14);
    gateReleaseSlider.setBounds(x, knobY + 12, 46, knobH);

    // 4. Bleed & MIDI
    x = 810;
    leakageEnableButton.setBounds(x, knobY + 6, 85, 20);
    leakLabel.setBounds(x + 85, knobY, 48, 14);
    leakageAmountSlider.setBounds(x + 85, knobY + 12, 48, knobH);

    midiOutButton.setBounds(x, knobY + 44, 80, 20);
    midiNoteLabel.setBounds(x + 85, knobY + 36, 48, 14);
    midiNoteSlider.setBounds(x + 85, knobY + 48, 48, 46);

    midiChLabel.setBounds(x + 135, knobY + 36, 42, 14);
    midiChannelSlider.setBounds(x + 135, knobY + 48, 42, 46);
}

} // namespace yeahwaah
