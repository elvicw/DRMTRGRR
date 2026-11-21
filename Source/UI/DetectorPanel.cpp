#include "DetectorPanel.h"
#include "SlateLookAndFeel.h"
#include <cmath>

namespace yeahwaah
{

DetectorPanel::DetectorPanel(juce::AudioProcessorValueTreeState& vts)
    : apvts(vts)
{
    // 1. Detection Engine Knobs (SSL White / Amber / Cyan / Green / Red)
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

    // 2. SSL Filter Section (SSL Blue Knobs)
    setupKnob(lowCutSlider, lowCutLabel, "LowCut", "LOW-CUT", "");
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
    auditionButton.setTooltip("Audition / Listen to filtered sidechain audio");
    addAndMakeVisible(auditionButton);
    auditionAtt = std::make_unique<ButtonAttachment>(apvts, "auditionFilter", auditionButton);

    // 3. SSL Transient Gate Section
    gateEnableButton.setClickingTogglesState(true);
    gateEnableButton.setTooltip("Engage Transient Noise Gate");
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

    // 4. Bleed Suppression & MIDI Section
    leakageEnableButton.setClickingTogglesState(true);
    leakageEnableButton.setTooltip("Engage Crosstalk / Bleed Suppression");
    addAndMakeVisible(leakageEnableButton);
    leakageEnableAtt = std::make_unique<ButtonAttachment>(apvts, "leakageEnable", leakageEnableButton);

    setupKnob(leakageAmountSlider, leakLabel, "LeakageAmount", "AMOUNT", "%");
    leakageAmountAtt = std::make_unique<SliderAttachment>(apvts, "leakageAmount", leakageAmountSlider);

    midiOutButton.setClickingTogglesState(true);
    midiOutButton.setTooltip("Enable MIDI Out triggering");
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
    s.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 58, 16);
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

    // Dark brushed slate chassis fill
    juce::ColourGradient bgGrad(juce::Colour(0xff181c25), 0.0f, 0.0f,
                                juce::Colour(0xff0e1016), 0.0f, static_cast<float>(h), false);
    g.setGradientFill(bgGrad);
    g.fillRect(0, 0, w, h);

    // Metallic frame
    g.setColour(SlateLookAndFeel::slateBorder);
    g.drawRect(0, 0, w, h, 1);

    // Section bounding boxes
    const int sec1W = 390;
    const int sec2W = 200;
    const int sec3W = 310;
    const int sec4W = w - sec1W - sec2W - sec3W;

    auto drawModule = [&](int mx, int mw, const juce::String& title, const juce::Colour& accent) {
        juce::Rectangle<float> mBox(static_cast<float>(mx) + 3.0f, 3.0f, static_cast<float>(mw) - 6.0f, static_cast<float>(h) - 6.0f);

        // Recessed module faceplate (SSL console sub-panel)
        g.setColour(juce::Colour(0xff13161e));
        g.fillRoundedRectangle(mBox, 3.5f);
        g.setColour(SlateLookAndFeel::slateBorder.darker(0.2f));
        g.drawRoundedRectangle(mBox, 3.5f, 1.0f);

        // Header plate
        juce::Rectangle<float> titleBox(mBox.getX(), mBox.getY(), mBox.getWidth(), 20.0f);
        g.setColour(juce::Colour(0xff1b1f2a));
        g.fillRoundedRectangle(titleBox, 3.0f);

        // Title text in bold silkscreen
        g.setFont(juce::Font(10.0f, juce::Font::bold));
        g.setColour(SlateLookAndFeel::textLight.withAlpha(0.95f));
        g.drawText(title, titleBox.toNearestInt().reduced(10, 0), juce::Justification::centredLeft);

        // Accent indicator LED on header
        g.setColour(accent);
        g.fillEllipse(titleBox.getRight() - 16.0f, titleBox.getCentreY() - 3.5f, 7.0f, 7.0f);
        g.setColour(juce::Colours::white);
        g.fillEllipse(titleBox.getRight() - 14.5f, titleBox.getCentreY() - 2.0f, 2.5f, 2.5f);
    };

    drawModule(0, sec1W, "TRIGGER ENGINE", SlateLookAndFeel::cyanGlow);
    drawModule(sec1W, sec2W, "BANDPASS FILTER", SlateLookAndFeel::sslBlue);
    drawModule(sec1W + sec2W, sec3W, "TRANSIENT GATE", SlateLookAndFeel::amberGlow);
    drawModule(sec1W + sec2W + sec3W, sec4W, "BLEED & MIDI", SlateLookAndFeel::cyanGlow);
}

void DetectorPanel::resized()
{
    const int knobY = 28;
    const int knobW = 68;
    const int knobH = 78;

    // 1. Transient Engine Knobs: Input, Detail, Sens, Retrig, Dyn (BIG SSL KNOBS)
    int x = 10;
    inputGainLabel.setBounds(x, knobY, knobW, 14);
    inputGainSlider.setBounds(x, knobY + 14, knobW, knobH);

    x += 74;
    detailLabel.setBounds(x, knobY, knobW, 14);
    detailSlider.setBounds(x, knobY + 14, knobW, knobH);

    x += 74;
    sensLabel.setBounds(x, knobY, knobW, 14);
    sensitivitySlider.setBounds(x, knobY + 14, knobW, knobH);

    x += 74;
    retriggerLabel.setBounds(x, knobY, knobW, 14);
    retriggerSlider.setBounds(x, knobY + 14, knobW, knobH);

    x += 74;
    dynLabel.setBounds(x, knobY, knobW, 14);
    dynamicsSlider.setBounds(x, knobY + 14, knobW, knobH);

    // 2. Filter: Low-Cut, High-Cut, Listen button
    x = 398;
    lowCutLabel.setBounds(x, knobY, 58, 14);
    lowCutSlider.setBounds(x, knobY + 14, 58, knobH);

    x += 62;
    highCutLabel.setBounds(x, knobY, 58, 14);
    highCutSlider.setBounds(x, knobY + 14, 58, knobH);

    auditionButton.setBounds(x + 64, knobY + 30, 62, 32);

    // 3. Gate: Enable, Thresh, Attack, Hold, Rel
    x = 598;
    gateEnableButton.setBounds(x, knobY + 32, 68, 28);

    x += 72;
    gateThreshLabel.setBounds(x, knobY, 52, 14);
    gateThresholdSlider.setBounds(x, knobY + 14, 52, knobH);

    x += 54;
    gateAttLabel.setBounds(x, knobY, 52, 14);
    gateAttackSlider.setBounds(x, knobY + 14, 52, knobH);

    x += 54;
    gateHldLabel.setBounds(x, knobY, 52, 14);
    gateHoldSlider.setBounds(x, knobY + 14, 52, knobH);

    x += 54;
    gateRelLabel.setBounds(x, knobY, 52, 14);
    gateReleaseSlider.setBounds(x, knobY + 14, 52, knobH);

    // 4. Bleed & MIDI
    x = 908;
    leakageEnableButton.setBounds(x + 6, knobY + 32, 78, 28);
    leakLabel.setBounds(x + 88, knobY, 50, 14);
    leakageAmountSlider.setBounds(x + 88, knobY + 14, 50, knobH);

    midiOutButton.setBounds(x + 144, knobY + 32, 72, 28);
    midiNoteLabel.setBounds(x + 220, knobY, 48, 14);
    midiNoteSlider.setBounds(x + 220, knobY + 14, 48, knobH);

    midiChLabel.setBounds(x + 272, knobY, 44, 14);
    midiChannelSlider.setBounds(x + 272, knobY + 14, 44, knobH);
}

} // namespace yeahwaah
