#include "SettingsView.h"
#include "SlateLookAndFeel.h"

namespace yeahwaah
{

SettingsView::SettingsView(DrmtrgrrAudioProcessor& proc)
    : processor(proc)
{
    headerLabel.setText("GLOBAL ENGINE SETTINGS & HARDWARE PREFERENCES", juce::dontSendNotification);
    headerLabel.setFont(juce::Font(13.5f, juce::Font::bold));
    headerLabel.setColour(juce::Label::textColourId, SlateLookAndFeel::textLight);
    addAndMakeVisible(headerLabel);

    versionLabel.setText("DRM-TRGRR v1.0.0  |  yeahwaah audio", juce::dontSendNotification);
    versionLabel.setFont(juce::Font(10.5f, juce::Font::bold));
    versionLabel.setColour(juce::Label::textColourId, SlateLookAndFeel::cyanGlow);
    addAndMakeVisible(versionLabel);

    velCurveLabel.setText("VELOCITY RESPONSE CURVE:", juce::dontSendNotification);
    velCurveLabel.setFont(juce::Font(10.0f, juce::Font::bold));
    velCurveLabel.setColour(juce::Label::textColourId, SlateLookAndFeel::textDim);
    addAndMakeVisible(velCurveLabel);

    velCurveBox.addItem("Linear (Default)", 1);
    velCurveBox.addItem("Logarithmic (Ghost Note Sensitivity)", 2);
    velCurveBox.addItem("Exponential (Punch Emphasis)", 3);
    velCurveBox.addItem("S-Curve (High Dynamic Contrast)", 4);
    velCurveBox.setSelectedId(1, juce::dontSendNotification);
    addAndMakeVisible(velCurveBox);

    juce::String details = "Plugin Format: VST3 / AU / Standalone Application (macOS & Windows 64-bit)\n"
                           "DSP Architecture: Phase-accurate 64-bit multi-voice polyphonic transient triggering\n"
                           "Interpolation: Fractional Hermite 4-point cubic pitch-shifting engine\n"
                           "Detection: Dual-stage sidechain bandpass filter with zero latency transient detector\n"
                           "Suppressor: Multi-mic leakage cancellation with transient-driven gate\n"
                           "Instrument Slots: 6 modular rack channels with velocity splitting & ADSR shaping\n"
                           "License: Activated / Platinum Edition (All Features Unlocked)\n\n"
                           "All rights reserved (c) yeahwaah audio.";

    infoText.setText(details, juce::dontSendNotification);
    infoText.setFont(juce::Font(10.0f, juce::Font::plain));
    infoText.setColour(juce::Label::textColourId, SlateLookAndFeel::textDim);
    addAndMakeVisible(infoText);

    restoreDefaultsButton.onClick = [this] {
        processor.applyFactoryPreset(0);
    };
    addAndMakeVisible(restoreDefaultsButton);
}

void SettingsView::paint(juce::Graphics& g)
{
    const int w = getWidth();
    const int h = getHeight();

    g.setColour(SlateLookAndFeel::bgDark);
    g.fillRect(0, 0, w, h);

    juce::Rectangle<float> panelBox(20.0f, 10.0f, static_cast<float>(w) - 40.0f, static_cast<float>(h) - 20.0f);
    g.setColour(SlateLookAndFeel::bgPanel);
    g.fillRoundedRectangle(panelBox, 4.0f);
    g.setColour(SlateLookAndFeel::slateBorder);
    g.drawRoundedRectangle(panelBox, 4.0f, 1.0f);

    // Decorative corner hex screws
    auto drawScrew = [&](float cx, float cy) {
        g.setColour(juce::Colour(0xff090b0e));
        g.fillEllipse(cx - 4.5f, cy - 4.5f, 9.0f, 9.0f);
        g.setColour(juce::Colour(0xff2f3745));
        g.drawEllipse(cx - 4.5f, cy - 4.5f, 9.0f, 9.0f, 1.0f);
    };
    drawScrew(panelBox.getX() + 10.0f, panelBox.getY() + 10.0f);
    drawScrew(panelBox.getRight() - 10.0f, panelBox.getY() + 10.0f);
    drawScrew(panelBox.getX() + 10.0f, panelBox.getBottom() - 10.0f);
    drawScrew(panelBox.getRight() - 10.0f, panelBox.getBottom() - 10.0f);
}

void SettingsView::resized()
{
    headerLabel.setBounds(40, 20, 600, 22);
    versionLabel.setBounds(40, 42, 600, 18);

    velCurveLabel.setBounds(40, 72, 200, 20);
    velCurveBox.setBounds(240, 72, 280, 24);

    infoText.setBounds(40, 108, getWidth() - 80, 110);
    restoreDefaultsButton.setBounds(40, 226, 220, 28);
}

} // namespace yeahwaah
