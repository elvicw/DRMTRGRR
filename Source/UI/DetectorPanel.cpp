#include "DetectorPanel.h"
#include "SlateLookAndFeel.h"
#include <cmath>

namespace yeahwaah
{

DetectorPanel::DetectorPanel(juce::AudioProcessorValueTreeState& vts)
    : apvts(vts)
{
    // 1. Detection Engine Knobs
    setupKnob(inputGainSlider, inputGainLabel, "InputTrim", "INPUT TRIM", " dB");
    inputGainAtt = std::make_unique<SliderAttachment>(apvts, "inputGain", inputGainSlider);

    setupKnob(detailSlider, detailLabel, "Detail", "DETAIL", " dB");
    detailAtt = std::make_unique<SliderAttachment>(apvts, "detail", detailSlider);

    setupKnob(sensitivitySlider, sensLabel, "Sensitivity", "SENSITIVITY", "%");
    sensAtt = std::make_unique<SliderAttachment>(apvts, "sensitivity", sensitivitySlider);

    setupKnob(retriggerSlider, retriggerLabel, "Retrigger", "RETRIGGER", " ms");
    retriggerAtt = std::make_unique<SliderAttachment>(apvts, "retrigger", retriggerSlider);

    setupKnob(dynamicsSlider, dynLabel, "Dynamics", "DYNAMICS", "%");
    dynAtt = std::make_unique<SliderAttachment>(apvts, "dynamics", dynamicsSlider);

    // 2. Filter Knobs
    setupKnob(lowCutSlider, lowCutLabel, "LowCut", "LOW-CUT", " Hz");
    lowCutSlider.textFromValueFunction = [](double val) {
        return juce::String(static_cast<int>(val)) + " Hz";
    };
    lowCutAtt = std::make_unique<SliderAttachment>(apvts, "lowCut", lowCutSlider);

    setupKnob(highCutSlider, highCutLabel, "HighCut", "HIGH-CUT", "");
    highCutSlider.textFromValueFunction = [](double val) {
        if (val >= 1000.0)
            return juce::String(val / 1000.0, 1) + " kHz";
        return juce::String(static_cast<int>(val)) + " Hz";
    };
    highCutAtt = std::make_unique<SliderAttachment>(apvts, "highCut", highCutSlider);

    auditionButton.setClickingTogglesState(true);
    addAndMakeVisible(auditionButton);
    auditionAtt = std::make_unique<ButtonAttachment>(apvts, "auditionFilter", auditionButton);

    // 3. Gate
    gateEnableButton.setClickingTogglesState(true);
    addAndMakeVisible(gateEnableButton);
    gateEnableAtt = std::make_unique<ButtonAttachment>(apvts, "gateEnable", gateEnableButton);

    setupKnob(gateThresholdSlider, gateThreshLabel, "GateThreshold", "THRESHOLD", " dB");
    gateThreshAtt = std::make_unique<SliderAttachment>(apvts, "gateThreshold", gateThresholdSlider);

    setupKnob(gateAttackSlider, gateAttLabel, "GateAttack", "ATTACK", " ms");
    gateAttAtt = std::make_unique<SliderAttachment>(apvts, "gateAttack", gateAttackSlider);

    setupKnob(gateHoldSlider, gateHldLabel, "GateHold", "HOLD", " ms");
    gateHldAtt = std::make_unique<SliderAttachment>(apvts, "gateHold", gateHoldSlider);

    setupKnob(gateReleaseSlider, gateRelLabel, "GateRelease", "RELEASE", " ms");
    gateRelAtt = std::make_unique<SliderAttachment>(apvts, "gateRelease", gateReleaseSlider);

    // 4. Leakage & MIDI
    leakageEnableButton.setClickingTogglesState(true);
    addAndMakeVisible(leakageEnableButton);
    leakageEnableAtt = std::make_unique<ButtonAttachment>(apvts, "leakageEnable", leakageEnableButton);

    setupKnob(leakageAmountSlider, leakLabel, "LeakageAmount", "AMOUNT", "%");
    leakageAmountAtt = std::make_unique<SliderAttachment>(apvts, "leakageAmount", leakageAmountSlider);

    midiOutButton.setClickingTogglesState(true);
    addAndMakeVisible(midiOutButton);
    midiOutAtt = std::make_unique<ButtonAttachment>(apvts, "midiOutEnable", midiOutButton);

    setupKnob(midiNoteSlider, midiNoteLabel, "MidiNote", "NOTE", "");
    midiNoteSlider.textFromValueFunction = [](double v) {
        int note = static_cast<int>(v);
        return juce::MidiMessage::getMidiNoteName(note, true, true, 3);
    };
    midiNoteAtt = std::make_unique<SliderAttachment>(apvts, "midiNote", midiNoteSlider);

    setupKnob(midiChannelSlider, midiChLabel, "MidiChannel", "CHANNEL", "");
    midiChannelAtt = std::make_unique<SliderAttachment>(apvts, "midiChannel", midiChannelSlider);
}

void DetectorPanel::setupKnob(juce::Slider& s, juce::Label& l, const juce::String& name, const juce::String& text, const juce::String& suffix)
{
    s.setName(name);
    s.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    s.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 52, 14);
    s.setTextValueSuffix(suffix);
    addAndMakeVisible(s);

    l.setText(text, juce::dontSendNotification);
    l.setFont(juce::Font(9.0f, juce::Font::bold));
    l.setJustificationType(juce::Justification::centred);
    l.setColour(juce::Label::textColourId, SlateLookAndFeel::textDim);
    addAndMakeVisible(l);
}

void DetectorPanel::paint(juce::Graphics& g)
{
    const int w = getWidth();
    const int h = getHeight();

    // Dark brushed slate chassis fill
    juce::ColourGradient bgGrad(juce::Colour(0xff181c24), 0.0f, 0.0f,
                                juce::Colour(0xff0e1015), 0.0f, static_cast<float>(h), false);
    g.setGradientFill(bgGrad);
    g.fillRect(0, 0, w, h);

    // Outer subtle metallic border
    g.setColour(SlateLookAndFeel::slateBorder);
    g.drawRect(0, 0, w, h, 1);

    // Section bounding boxes
    const int sec1W = 345;
    const int sec2W = 180;
    const int sec3W = 270;
    const int sec4W = w - sec1W - sec2W - sec3W;

    auto drawModule = [&](int mx, int mw, const juce::String& title, const juce::Colour& accent) {
        juce::Rectangle<float> mBox(static_cast<float>(mx) + 2.0f, 2.0f, static_cast<float>(mw) - 4.0f, static_cast<float>(h) - 4.0f);

        // Recessed module faceplate
        g.setColour(juce::Colour(0xff14171f));
        g.fillRoundedRectangle(mBox, 3.0f);
        g.setColour(SlateLookAndFeel::slateBorder.darker(0.3f));
        g.drawRoundedRectangle(mBox, 3.0f, 1.0f);

        // Header plate
        juce::Rectangle<float> titleBox(mBox.getX(), mBox.getY(), mBox.getWidth(), 18.0f);
        g.setColour(juce::Colour(0xff1a1e28));
        g.fillRoundedRectangle(titleBox, 3.0f);

        // Title text
        g.setFont(juce::Font(9.5f, juce::Font::bold));
        g.setColour(SlateLookAndFeel::textLight.withAlpha(0.9f));
        g.drawText(title, titleBox.toNearestInt().reduced(8, 0), juce::Justification::centredLeft);

        // Accent indicator dot on header
        g.setColour(accent);
        g.fillEllipse(titleBox.getRight() - 14.0f, titleBox.getCentreY() - 3.0f, 6.0f, 6.0f);
    };

    drawModule(0, sec1W, "TRIGGER ENGINE", SlateLookAndFeel::cyanGlow);
    drawModule(sec1W, sec2W, "BANDPASS FILTER", juce::Colour(0xff00ffaa));
    drawModule(sec1W + sec2W, sec3W, "TRANSIENT GATE", SlateLookAndFeel::amberGlow);
    drawModule(sec1W + sec2W + sec3W, sec4W, "BLEED & MIDI", SlateLookAndFeel::cyanGlow);
}

void DetectorPanel::resized()
{
    const int knobY = 22;
    const int knobW = 58;
    const int knobH = 64;

    // 1. Transient Engine Knobs: Input, Detail, Sens, Retrig, Dyn
    int x = 8;
    inputGainLabel.setBounds(x, knobY, knobW, 12);
    inputGainSlider.setBounds(x, knobY + 12, knobW, knobH);

    x += 66;
    detailLabel.setBounds(x, knobY, knobW, 12);
    detailSlider.setBounds(x, knobY + 12, knobW, knobH);

    x += 66;
    sensLabel.setBounds(x, knobY, knobW, 12);
    sensitivitySlider.setBounds(x, knobY + 12, knobW, knobH);

    x += 66;
    retriggerLabel.setBounds(x, knobY, knobW, 12);
    retriggerSlider.setBounds(x, knobY + 12, knobW, knobH);

    x += 66;
    dynLabel.setBounds(x, knobY, knobW, 12);
    dynamicsSlider.setBounds(x, knobY + 12, knobW, knobH);

    // 2. Filter: Low-Cut, High-Cut, Audition button
    x = 352;
    lowCutLabel.setBounds(x, knobY, 54, 12);
    lowCutSlider.setBounds(x, knobY + 12, 54, knobH);

    x += 58;
    highCutLabel.setBounds(x, knobY, 54, 12);
    highCutSlider.setBounds(x, knobY + 12, 54, knobH);

    auditionButton.setBounds(x + 58, knobY + 22, 52, 30);

    // 3. Gate: Enable, Thresh, Attack, Hold, Rel
    x = 532;
    gateEnableButton.setBounds(x, knobY + 24, 62, 24);

    x += 64;
    gateThreshLabel.setBounds(x, knobY, 46, 12);
    gateThresholdSlider.setBounds(x, knobY + 12, 46, knobH);

    x += 48;
    gateAttLabel.setBounds(x, knobY, 46, 12);
    gateAttackSlider.setBounds(x, knobY + 12, 46, knobH);

    x += 48;
    gateHldLabel.setBounds(x, knobY, 46, 12);
    gateHoldSlider.setBounds(x, knobY + 12, 46, knobH);

    x += 48;
    gateRelLabel.setBounds(x, knobY, 46, 12);
    gateReleaseSlider.setBounds(x, knobY + 12, 46, knobH);

    // 4. Bleed & MIDI
    x = 800;
    leakageEnableButton.setBounds(x + 4, knobY + 20, 72, 22);
    leakLabel.setBounds(x + 78, knobY, 44, 12);
    leakageAmountSlider.setBounds(x + 78, knobY + 12, 44, knobH);

    midiOutButton.setBounds(x + 126, knobY + 20, 64, 22);
    midiNoteLabel.setBounds(x + 192, knobY, 40, 12);
    midiNoteSlider.setBounds(x + 192, knobY + 12, 40, knobH);

    midiChLabel.setBounds(x + 236, knobY, 36, 12);
    midiChannelSlider.setBounds(x + 236, knobY + 12, 36, knobH);
}

} // namespace yeahwaah
