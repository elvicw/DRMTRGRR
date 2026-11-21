#include "HeaderBar.h"
#include "SlateLookAndFeel.h"
#include "../PluginProcessor.h"

namespace yeahwaah
{

HeaderBar::HeaderBar(DrmtrgrrAudioProcessor& proc)
    : processor(proc)
{
    // Preset dropdown
    for (int i = 0; i < processor.getNumPrograms(); ++i)
        presetBox.addItem(processor.getProgramName(i), i + 1);

    presetBox.setSelectedId(processor.getCurrentProgram() + 1, juce::dontSendNotification);
    presetBox.onChange = [this] {
        int id = presetBox.getSelectedId() - 1;
        if (id >= 0)
            processor.setCurrentProgram(id);
    };
    addAndMakeVisible(presetBox);

    // Dry / Wet Mix Knob
    mixSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    mixSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 48, 14);
    mixSlider.setTextValueSuffix("%");
    addAndMakeVisible(mixSlider);
    mixAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processor.getAPVTS(), "mix", mixSlider);

    mixLabel.setText("MIX", juce::dontSendNotification);
    mixLabel.setFont(juce::Font(9.5f, juce::Font::bold));
    mixLabel.setColour(juce::Label::textColourId, SlateLookAndFeel::textDim);
    mixLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(mixLabel);

    // Master Output Level Knob
    outputGainSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    outputGainSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 48, 14);
    outputGainSlider.setTextValueSuffix(" dB");
    addAndMakeVisible(outputGainSlider);
    outGainAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        processor.getAPVTS(), "outputGain", outputGainSlider);

    outLabel.setText("OUTPUT", juce::dontSendNotification);
    outLabel.setFont(juce::Font(9.5f, juce::Font::bold));
    outLabel.setColour(juce::Label::textColourId, SlateLookAndFeel::textDim);
    outLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(outLabel);
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

    // Brushed dark slate top bar
    juce::ColourGradient bgGrad(juce::Colour(0xff232730), 0.0f, 0.0f,
                                juce::Colour(0xff14161a), 0.0f, (float)h, false);
    g.setGradientFill(bgGrad);
    g.fillRect(0, 0, w, h);

    // Bottom hairline separator
    g.setColour(SlateLookAndFeel::slateBorder);
    g.drawHorizontalLine(h - 1, 0.0f, (float)w);

    // Company Logo ("yeahwaah")
    g.setFont(juce::Font(12.0f, juce::Font::bold));
    g.setColour(SlateLookAndFeel::cyanGlow.withAlpha(0.9f));
    g.drawText("YEAHWAAH", 16, 6, 120, 14, juce::Justification::left);

    // Plugin Name ("DRMTRGRR")
    g.setFont(juce::Font(22.0f, juce::Font::bold));
    // Subtle 3D shadow
    g.setColour(juce::Colour(0xff08090b));
    g.drawText("DRMTRGRR", 17, 21, 180, 26, juce::Justification::left);

    // Glowing embossed text
    g.setColour(SlateLookAndFeel::textLight);
    g.drawText("DRMTRGRR", 16, 20, 180, 26, juce::Justification::left);

    // Tagline / Version
    g.setFont(juce::Font(9.0f, juce::Font::plain));
    g.setColour(SlateLookAndFeel::textDim);
    g.drawText("DRUM TRANSIENT REPLACER & ENHANCER  |  VST3", 16, 44, 280, 12, juce::Justification::left);

    // Input Meter (Beside preset box)
    const float inMeterX = 490.0f;
    const float meterY = 12.0f;
    const float meterW = 7.0f;
    const float meterH = 34.0f;

    g.setFont(juce::Font(8.0f, juce::Font::bold));
    g.setColour(SlateLookAndFeel::textDim);
    g.drawText("IN", (int)inMeterX - 14, (int)meterY + 10, 12, 10, juce::Justification::centredRight);

    g.setColour(juce::Colour(0xff0c0e11));
    g.fillRoundedRectangle(inMeterX, meterY, meterW, meterH, 1.5f);
    if (currentInPeak > 0.001f)
    {
        float p = juce::jlimit(0.0f, 1.0f, currentInPeak);
        float activeH = p * meterH;
        g.setColour(p > 0.95f ? SlateLookAndFeel::muteRed : SlateLookAndFeel::cyanGlow);
        g.fillRoundedRectangle(inMeterX + 1.0f, meterY + meterH - activeH, meterW - 2.0f, activeH, 1.0f);
    }

    // Output Meter
    const float outMeterX = (float)w - 24.0f;
    g.drawText("OUT", (int)outMeterX - 18, (int)meterY + 10, 16, 10, juce::Justification::centredRight);

    g.setColour(juce::Colour(0xff0c0e11));
    g.fillRoundedRectangle(outMeterX, meterY, meterW, meterH, 1.5f);
    if (currentOutPeak > 0.001f)
    {
        float p = juce::jlimit(0.0f, 1.0f, currentOutPeak);
        float activeH = p * meterH;
        g.setColour(p > 0.95f ? SlateLookAndFeel::muteRed : SlateLookAndFeel::cyanGlow);
        g.fillRoundedRectangle(outMeterX + 1.0f, meterY + meterH - activeH, meterW - 2.0f, activeH, 1.0f);
    }
}

void HeaderBar::resized()
{
    const int w = getWidth();

    // Preset dropdown in center
    presetBox.setBounds(310, 16, 170, 24);

    // Mix & Output Knobs on right
    mixLabel.setBounds(w - 180, 4, 48, 12);
    mixSlider.setBounds(w - 180, 14, 48, 44);

    outLabel.setBounds(w - 115, 4, 48, 12);
    outputGainSlider.setBounds(w - 115, 14, 48, 44);
}

} // namespace yeahwaah
