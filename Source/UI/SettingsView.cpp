#include "SettingsView.h"
#include "SlateLookAndFeel.h"

namespace yeahwaah
{

SettingsView::SettingsView(DrmtrgrrAudioProcessor& proc)
    : processor(proc)
{
    headerLabel.setText("GLOBAL PLUGIN SETTINGS & PREFERENCES", juce::dontSendNotification);
    headerLabel.setFont(juce::Font(14.0f, juce::Font::bold));
    headerLabel.setColour(juce::Label::textColourId, SlateLookAndFeel::textLight);
    addAndMakeVisible(headerLabel);

    versionLabel.setText("DRMTRGRR v1.0.0  |  COMPANY: yeahwaah", juce::dontSendNotification);
    versionLabel.setFont(juce::Font(11.0f, juce::Font::bold));
    versionLabel.setColour(juce::Label::textColourId, SlateLookAndFeel::cyanGlow);
    addAndMakeVisible(versionLabel);

    juce::String details = "Plugin Format: VST3 / AU / Standalone\n"
                           "Engine: Phase-accurate 64-bit multi-voice drum transient triggering\n"
                           "Polyphony: 4 voices per channel with fractional Hermite pitch interpolation\n"
                           "Features: Slate Trigger 2 detection architecture, transient bandpass filter,\n"
                           "leakage suppression, transient-driven noise gate, full MIDI I/O,\n"
                           "multi-channel rack mixer with velocity splitting and ADSR shaping.\n\n"
                           "All rights reserved (c) yeahwaah.";

    infoText.setText(details, juce::dontSendNotification);
    infoText.setFont(juce::Font(10.5f, juce::Font::plain));
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

    juce::Rectangle<float> panelBox(24.0f, 16.0f, (float)w - 48.0f, (float)h - 32.0f);
    g.setColour(SlateLookAndFeel::bgPanel);
    g.fillRoundedRectangle(panelBox, 4.0f);
    g.setColour(SlateLookAndFeel::slateBorder);
    g.drawRoundedRectangle(panelBox, 4.0f, 1.0f);
}

void SettingsView::resized()
{
    headerLabel.setBounds(40, 28, 400, 24);
    versionLabel.setBounds(40, 54, 400, 20);
    infoText.setBounds(40, 85, getWidth() - 80, 140);
    restoreDefaultsButton.setBounds(40, 240, 220, 30);
}

} // namespace yeahwaah
