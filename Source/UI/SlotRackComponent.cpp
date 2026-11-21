#include "SlotRackComponent.h"
#include "SlateLookAndFeel.h"
#include "../PluginProcessor.h"

namespace yeahwaah
{

SingleSlotStrip::SingleSlotStrip(int index, DrmtrgrrAudioProcessor& proc)
    : slotIndex(index), processor(proc), slot(proc.getSlot(index))
{
    juce::String prefix = "slot_" + juce::String(slotIndex) + "_";
    auto& apvts = processor.getAPVTS();

    // Enable button
    enableButton.setClickingTogglesState(true);
    addAndMakeVisible(enableButton);
    enableAtt = std::make_unique<ButtonAttachment>(apvts, prefix + "enable", enableButton);

    // Name label
    nameLabel.setText(slot.getSampleName(), juce::dontSendNotification);
    nameLabel.setFont(juce::Font(10.5f, juce::Font::bold));
    nameLabel.setColour(juce::Label::textColourId, SlateLookAndFeel::textLight);
    nameLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(nameLabel);

    // Buttons
    loadButton.onClick = [this] { chooseSampleFile(); };
    addAndMakeVisible(loadButton);

    factoryButton.onClick = [this] { cycleFactoryDrum(); };
    addAndMakeVisible(factoryButton);

    testHitButton.onClick = [this] { processor.triggerSlotManually(slotIndex, 1.0f); };
    addAndMakeVisible(testHitButton);

    // Volume Slider (Vertical Fader)
    volumeSlider.setSliderStyle(juce::Slider::LinearVertical);
    volumeSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 48, 14);
    volumeSlider.setTextValueSuffix(" dB");
    addAndMakeVisible(volumeSlider);
    volumeAtt = std::make_unique<SliderAttachment>(apvts, prefix + "volume", volumeSlider);

    // Pan & Tune Knobs
    panSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    panSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 42, 14);
    addAndMakeVisible(panSlider);
    panAtt = std::make_unique<SliderAttachment>(apvts, prefix + "pan", panSlider);

    tuneSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    tuneSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 44, 14);
    tuneSlider.setTextValueSuffix(" st");
    addAndMakeVisible(tuneSlider);
    tuneAtt = std::make_unique<SliderAttachment>(apvts, prefix + "tune", tuneSlider);

    // Solo, Mute, Phase Buttons
    soloButton.setClickingTogglesState(true);
    addAndMakeVisible(soloButton);
    soloAtt = std::make_unique<ButtonAttachment>(apvts, prefix + "solo", soloButton);

    muteButton.setClickingTogglesState(true);
    addAndMakeVisible(muteButton);
    muteAtt = std::make_unique<ButtonAttachment>(apvts, prefix + "mute", muteButton);

    phaseButton.setClickingTogglesState(true);
    addAndMakeVisible(phaseButton);
    phaseAtt = std::make_unique<ButtonAttachment>(apvts, prefix + "phase", phaseButton);

    // Envelope & Velocity mini sliders
    attackSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    attackSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    attackSlider.setTooltip("Attack");
    addAndMakeVisible(attackSlider);
    attackAtt = std::make_unique<SliderAttachment>(apvts, prefix + "attack", attackSlider);

    decaySlider.setSliderStyle(juce::Slider::LinearHorizontal);
    decaySlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    decaySlider.setTooltip("Decay");
    addAndMakeVisible(decaySlider);
    decayAtt = std::make_unique<SliderAttachment>(apvts, prefix + "decay", decaySlider);

    minVelSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    minVelSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    minVelSlider.setTooltip("Min Velocity");
    addAndMakeVisible(minVelSlider);
    minVelAtt = std::make_unique<SliderAttachment>(apvts, prefix + "minVel", minVelSlider);

    maxVelSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    maxVelSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    maxVelSlider.setTooltip("Max Velocity");
    addAndMakeVisible(maxVelSlider);
    maxVelAtt = std::make_unique<SliderAttachment>(apvts, prefix + "maxVel", maxVelSlider);

    // Labels
    volLabel.setText("VOL", juce::dontSendNotification);
    volLabel.setFont(juce::Font(9.0f, juce::Font::bold));
    volLabel.setColour(juce::Label::textColourId, SlateLookAndFeel::textDim);
    volLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(volLabel);

    panLabel.setText("PAN", juce::dontSendNotification);
    panLabel.setFont(juce::Font(9.0f, juce::Font::bold));
    panLabel.setColour(juce::Label::textColourId, SlateLookAndFeel::textDim);
    panLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(panLabel);

    tuneLabel.setText("TUNE", juce::dontSendNotification);
    tuneLabel.setFont(juce::Font(9.0f, juce::Font::bold));
    tuneLabel.setColour(juce::Label::textColourId, SlateLookAndFeel::textDim);
    tuneLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(tuneLabel);

    slot.getWaveformThumbnail(waveformThumbnail, 48);
}

void SingleSlotStrip::chooseSampleFile()
{
    fileChooser = std::make_unique<juce::FileChooser>(
        "Select Drum Sample", juce::File::getSpecialLocation(juce::File::userHomeDirectory),
        "*.wav;*.aif;*.aiff;*.mp3;*.flac");

    auto flags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles;
    fileChooser->launchAsync(flags, [this](const juce::FileChooser& fc) {
        auto result = fc.getResult();
        if (result.existsAsFile())
        {
            if (slot.loadFromFile(result, processor.getFormatManager()))
            {
                nameLabel.setText(slot.getSampleName(), juce::dontSendNotification);
                slot.getWaveformThumbnail(waveformThumbnail, 48);
                repaint();
            }
        }
    });
}

void SingleSlotStrip::cycleFactoryDrum()
{
    static int cycleCounter = 0;
    cycleCounter = (cycleCounter + 1) % static_cast<int>(FactoryDrumType::Count);
    slot.loadFactoryDrum(static_cast<FactoryDrumType>(cycleCounter));
    nameLabel.setText(slot.getSampleName(), juce::dontSendNotification);
    slot.getWaveformThumbnail(waveformThumbnail, 48);
    repaint();
}

bool SingleSlotStrip::isInterestedInFileDrag(const juce::StringArray& files)
{
    for (const auto& f : files)
    {
        if (f.endsWithIgnoreCase(".wav") || f.endsWithIgnoreCase(".aif") ||
            f.endsWithIgnoreCase(".aiff") || f.endsWithIgnoreCase(".mp3") ||
            f.endsWithIgnoreCase(".flac"))
            return true;
    }
    return false;
}

void SingleSlotStrip::fileDragEnter(const juce::StringArray& /*files*/, int /*x*/, int /*y*/)
{
    isDragHovered = true;
    repaint();
}

void SingleSlotStrip::fileDragExit(const juce::StringArray& /*files*/)
{
    isDragHovered = false;
    repaint();
}

void SingleSlotStrip::filesDropped(const juce::StringArray& files, int /*x*/, int /*y*/)
{
    isDragHovered = false;
    if (!files.isEmpty())
    {
        juce::File f(files[0]);
        if (slot.loadFromFile(f, processor.getFormatManager()))
        {
            nameLabel.setText(slot.getSampleName(), juce::dontSendNotification);
            slot.getWaveformThumbnail(waveformThumbnail, 48);
            repaint();
        }
    }
}

void SingleSlotStrip::updateMeterAndWaveform()
{
    currentPeak = slot.getCurrentPeakMeter();
    if (nameLabel.getText() != slot.getSampleName())
    {
        nameLabel.setText(slot.getSampleName(), juce::dontSendNotification);
        slot.getWaveformThumbnail(waveformThumbnail, 48);
    }
    repaint();
}

void SingleSlotStrip::paint(juce::Graphics& g)
{
    const int w = getWidth();
    const int h = getHeight();

    // Strip background
    g.setColour(SlateLookAndFeel::bgDark.brighter(0.04f));
    g.fillRoundedRectangle(2.0f, 2.0f, (float)w - 4.0f, (float)h - 4.0f, 4.0f);
    g.setColour(SlateLookAndFeel::slateBorder);
    g.drawRoundedRectangle(2.0f, 2.0f, (float)w - 4.0f, (float)h - 4.0f, 4.0f, 1.0f);

    // Strip header bar
    g.setColour(SlateLookAndFeel::bgPanel);
    g.fillRect(3, 3, w - 6, 22);
    g.setColour(SlateLookAndFeel::textLight);
    g.setFont(juce::Font(10.0f, juce::Font::bold));
    g.drawText("SLOT " + juce::String(slotIndex + 1), 22, 3, 50, 22, juce::Justification::left);

    // Mini Waveform Display Box
    juce::Rectangle<float> waveBox(6.0f, 48.0f, (float)w - 12.0f, 32.0f);
    g.setColour(juce::Colour(0xff0e1014));
    g.fillRoundedRectangle(waveBox, 3.0f);
    g.setColour(SlateLookAndFeel::slateBorder);
    g.drawRoundedRectangle(waveBox, 3.0f, 1.0f);

    // Render waveform thumbnail (Neon Tube Glow)
    if (!waveformThumbnail.empty())
    {
        const int numPts = (int)waveformThumbnail.size();
        const float step = (waveBox.getWidth() - 4.0f) / static_cast<float>(numPts);
        const float midY = waveBox.getCentreY();

        for (int i = 0; i < numPts; ++i)
        {
            float peakH = waveformThumbnail[static_cast<size_t>(i)] * (waveBox.getHeight() * 0.44f);
            float px = waveBox.getX() + 2.0f + static_cast<float>(i) * step;

            // Outer cyan glow line
            g.setColour(SlateLookAndFeel::cyanGlow.withAlpha(0.35f));
            g.drawVerticalLine(static_cast<int>(px), midY - peakH - 1.0f, midY + peakH + 1.0f);

            // Core electric line
            g.setColour(juce::Colour(0xffb3ffff));
            g.drawVerticalLine(static_cast<int>(px), midY - peakH, midY + peakH);
        }

        // Active hit flash playhead across waveform when sample triggers
        if (currentPeak > 0.05f)
        {
            g.setColour(juce::Colours::white.withAlpha(std::min(1.0f, currentPeak)));
            g.fillRect(waveBox.getX() + 3.0f, waveBox.getY() + 2.0f, 2.5f, waveBox.getHeight() - 4.0f);
        }
    }

    // 10-Segment Studio LED Ladder Meter beside volume fader
    const float meterX = 52.0f;
    const float meterY = 114.0f;
    const float meterW = 7.0f;
    const int numLeds = 10;
    const float ledH = 8.5f;
    const float ledSpacing = 1.5f;

    float normPeak = juce::jlimit(0.0f, 1.0f, currentPeak);
    int litLeds = static_cast<int>(normPeak * numLeds);

    for (int l = 0; l < numLeds; ++l)
    {
        float ly = meterY + (numLeds - 1 - l) * (ledH + ledSpacing);
        bool isLit = (l < litLeds);

        juce::Colour ledColor;
        if (l >= 8) // Red clip LEDs
            ledColor = juce::Colour(0xffff2d55);
        else if (l >= 6) // Amber warning LEDs
            ledColor = juce::Colour(0xffffa600);
        else // Green nominal LEDs
            ledColor = juce::Colour(0xff00e676);

        if (isLit)
        {
            // Lit LED glow
            g.setColour(ledColor.withAlpha(0.3f));
            g.fillRoundedRectangle(meterX - 1.0f, ly - 1.0f, meterW + 2.0f, ledH + 2.0f, 1.5f);
            g.setColour(ledColor);
            g.fillRoundedRectangle(meterX, ly, meterW, ledH, 1.0f);
        }
        else
        {
            // Dark unlit translucent LED lens
            g.setColour(ledColor.withAlpha(0.12f));
            g.fillRoundedRectangle(meterX, ly, meterW, ledH, 1.0f);
            g.setColour(juce::Colour(0xff0b0d11));
            g.drawRoundedRectangle(meterX, ly, meterW, ledH, 1.0f, 0.6f);
        }
    }

    // Mini parameter labels
    g.setFont(juce::Font(8.0f, juce::Font::bold));
    g.setColour(SlateLookAndFeel::textDim);
    g.drawText("ATT / DEC", 10, h - 54, w - 20, 10, juce::Justification::centred);
    g.drawText("VEL RANGE", 10, h - 28, w - 20, 10, juce::Justification::centred);

    // Drag-and-drop hover glow overlay
    if (isDragHovered)
    {
        g.setColour(SlateLookAndFeel::cyanGlow.withAlpha(0.25f));
        g.fillRoundedRectangle(2.0f, 2.0f, (float)w - 4.0f, (float)h - 4.0f, 4.0f);
        g.setColour(SlateLookAndFeel::cyanGlow);
        g.drawRoundedRectangle(2.0f, 2.0f, (float)w - 4.0f, (float)h - 4.0f, 4.0f, 2.0f);

        g.setFont(juce::Font(12.5f, juce::Font::bold));
        g.setColour(juce::Colours::white);
        g.drawText("DROP SAMPLE", 0, h / 2 - 12, w, 24, juce::Justification::centred);
    }
}

void SingleSlotStrip::resized()
{
    const int w = getWidth();
    const int h = getHeight();

    enableButton.setBounds(6, 4, 16, 16);
    nameLabel.setBounds(6, 26, w - 12, 18);

    // Row of Action buttons
    loadButton.setBounds(6, 84, 46, 20);
    factoryButton.setBounds(56, 84, 52, 20);
    testHitButton.setBounds(112, 84, 38, 20);

    // Fader & Meter
    volLabel.setBounds(12, 106, 36, 12);
    volumeSlider.setBounds(6, 118, 44, 110);

    // Pan & Tune Knobs
    panLabel.setBounds(66, 110, 42, 12);
    panSlider.setBounds(66, 122, 42, 48);

    tuneLabel.setBounds(110, 110, 42, 12);
    tuneSlider.setBounds(110, 122, 42, 48);

    // Solo, Mute, Phase
    soloButton.setBounds(66, 180, 26, 20);
    muteButton.setBounds(96, 180, 26, 20);
    phaseButton.setBounds(126, 180, 26, 20);

    // Envelopes
    attackSlider.setBounds(8, h - 44, (w - 18) / 2, 14);
    decaySlider.setBounds(10 + (w - 18) / 2, h - 44, (w - 18) / 2, 14);

    // Velocity Range
    minVelSlider.setBounds(8, h - 18, (w - 18) / 2, 14);
    maxVelSlider.setBounds(10 + (w - 18) / 2, h - 18, (w - 18) / 2, 14);
}

// SlotRackComponent
SlotRackComponent::SlotRackComponent(DrmtrgrrAudioProcessor& proc)
{
    for (int i = 0; i < DrmtrgrrAudioProcessor::NumSlots; ++i)
    {
        auto strip = std::make_unique<SingleSlotStrip>(i, proc);
        addAndMakeVisible(*strip);
        slotStrips.push_back(std::move(strip));
    }
}

void SlotRackComponent::updateMeters()
{
    for (auto& s : slotStrips)
        s->updateMeterAndWaveform();
}

void SlotRackComponent::paint(juce::Graphics& g)
{
    g.setColour(SlateLookAndFeel::bgDark);
    g.fillAll();
}

void SlotRackComponent::resized()
{
    const int w = getWidth();
    const int h = getHeight();
    const int numStrips = (int)slotStrips.size();
    if (numStrips == 0) return;

    const int stripW = w / numStrips;
    for (int i = 0; i < numStrips; ++i)
    {
        slotStrips[i]->setBounds(i * stripW, 0, stripW, h);
    }
}

} // namespace yeahwaah
