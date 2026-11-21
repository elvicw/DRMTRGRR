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

    // Analog Hardware Screws in top left and right
    auto drawRackScrew = [&](float cx, float cy) {
        g.setColour(juce::Colour(0xff090b0e));
        g.fillEllipse(cx - 5.5f, cy - 5.5f, 11.0f, 11.0f);
        juce::ColourGradient screwGrad(juce::Colour(0xff4a5360), cx - 2.0f, cy - 2.0f,
                                       juce::Colour(0xff181b20), cx + 3.0f, cy + 3.0f, true);
        g.setGradientFill(screwGrad);
        g.fillEllipse(cx - 4.5f, cy - 4.5f, 9.0f, 9.0f);
        g.setColour(juce::Colour(0xff0b0d11));
        g.drawLine(cx - 3.0f, cy, cx + 3.0f, cy, 1.2f);
        g.drawLine(cx, cy - 3.0f, cx, cy + 3.0f, 1.2f);
    };
    drawRackScrew(8.0f, 8.0f);
    drawRackScrew((float)w - 8.0f, 8.0f);

    // Company Logo ("yeahwaah")
    g.setFont(juce::Font(11.5f, juce::Font::bold));
    g.setColour(SlateLookAndFeel::cyanGlow);
    g.drawText("YEAHWAAH", 24, 6, 120, 14, juce::Justification::left);

    // Plugin Name ("DRMTRGRR")
    g.setFont(juce::Font(23.0f, juce::Font::bold));
    // 3D Shadow
    g.setColour(juce::Colour(0xff06070a));
    g.drawText("DRMTRGRR", 25, 21, 180, 26, juce::Justification::left);

    // Glowing embossed text
    g.setColour(SlateLookAndFeel::textLight);
    g.drawText("DRMTRGRR", 24, 20, 180, 26, juce::Justification::left);

    // Glowing underline accent bar under DRMTRGRR
    g.setColour(SlateLookAndFeel::cyanGlow.withAlpha(0.7f));
    g.fillRect(24.0f, 44.0f, 118.0f, 2.0f);
    g.setColour(SlateLookAndFeel::cyanGlow.withAlpha(0.2f));
    g.fillRect(24.0f, 43.0f, 118.0f, 4.0f);

    // Tagline
    g.setFont(juce::Font(8.5f, juce::Font::bold));
    g.setColour(SlateLookAndFeel::textDim);
    g.drawText("PRO TRANSIENT REPLACER  |  VST3", 148, 40, 170, 12, juce::Justification::left);

    // Multi-Segment Input Meter
    const float inMeterX = 492.0f;
    const float meterY = 12.0f;
    const float meterW = 7.0f;
    const int numMtrLeds = 6;
    const float ledH = 4.5f;

    g.setFont(juce::Font(8.0f, juce::Font::bold));
    g.setColour(SlateLookAndFeel::textDim);
    g.drawText("IN", (int)inMeterX - 14, (int)meterY + 10, 12, 10, juce::Justification::centredRight);

    int inLedsLit = static_cast<int>(juce::jlimit(0.0f, 1.0f, currentInPeak) * numMtrLeds);
    for (int l = 0; l < numMtrLeds; ++l)
    {
        float ly = meterY + (numMtrLeds - 1 - l) * (ledH + 1.2f);
        bool isLit = (l < inLedsLit);
        juce::Colour c = (l >= 5) ? SlateLookAndFeel::muteRed : (l >= 4) ? SlateLookAndFeel::amberGlow : SlateLookAndFeel::cyanGlow;
        g.setColour(isLit ? c : c.withAlpha(0.12f));
        g.fillRoundedRectangle(inMeterX, ly, meterW, ledH, 1.0f);
    }

    // Multi-Segment Output Meter
    const float outMeterX = (float)w - 24.0f;
    g.drawText("OUT", (int)outMeterX - 18, (int)meterY + 10, 16, 10, juce::Justification::centredRight);

    int outLedsLit = static_cast<int>(juce::jlimit(0.0f, 1.0f, currentOutPeak) * numMtrLeds);
    for (int l = 0; l < numMtrLeds; ++l)
    {
        float ly = meterY + (numMtrLeds - 1 - l) * (ledH + 1.2f);
        bool isLit = (l < outLedsLit);
        juce::Colour c = (l >= 5) ? SlateLookAndFeel::muteRed : (l >= 4) ? SlateLookAndFeel::amberGlow : SlateLookAndFeel::cyanGlow;
        g.setColour(isLit ? c : c.withAlpha(0.12f));
        g.fillRoundedRectangle(outMeterX, ly, meterW, ledH, 1.0f);
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
