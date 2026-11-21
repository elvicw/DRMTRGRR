#include "SlotRackComponent.h"
#include "SlateLookAndFeel.h"
#include "../PluginProcessor.h"
#include <cmath>

namespace yeahwaah
{

SingleSlotStrip::SingleSlotStrip(int index, DrmtrgrrAudioProcessor& proc)
    : slotIndex(index), processor(proc), slot(proc.getSlot(index)),
      slotThemeColor(SlateLookAndFeel::getSlotColor(index))
{
    juce::String prefix = "slot_" + juce::String(slotIndex) + "_";
    auto& apvts = processor.getAPVTS();

    // 1. Channel Enable Button
    enableButton.setClickingTogglesState(true);
    enableButton.setTooltip("Enable Channel");
    addAndMakeVisible(enableButton);
    enableAtt = std::make_unique<ButtonAttachment>(apvts, prefix + "enable", enableButton);

    // 2. Name label
    nameLabel.setText(slot.getSampleName().isEmpty() ? "EMPTY" : slot.getSampleName(), juce::dontSendNotification);
    nameLabel.setFont(juce::Font(10.5f, juce::Font::bold));
    nameLabel.setColour(juce::Label::textColourId, SlateLookAndFeel::textLight);
    nameLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(nameLabel);

    // 3. Quick Action Buttons
    loadButton.setTooltip("Load audio file");
    loadButton.onClick = [this] { chooseSampleFile(); };
    addAndMakeVisible(loadButton);

    factoryButton.setTooltip("Cycle built-in drum models");
    factoryButton.onClick = [this] { cycleFactoryDrum(); };
    addAndMakeVisible(factoryButton);

    testHitButton.setTooltip("Trigger test audition hit");
    testHitButton.onClick = [this] { processor.triggerSlotManually(slotIndex, 1.0f); };
    addAndMakeVisible(testHitButton);

    // 4. Volume Fader (Console Vertical Slider)
    volumeSlider.setName("SlotVolume");
    volumeSlider.setSliderStyle(juce::Slider::LinearVertical);
    volumeSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 48, 14);
    volumeSlider.setTextValueSuffix(" dB");
    addAndMakeVisible(volumeSlider);
    volumeAtt = std::make_unique<SliderAttachment>(apvts, prefix + "volume", volumeSlider);

    // 5. Pan & Tune Knobs
    panSlider.setName("SlotPan");
    panSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    panSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 42, 14);
    panSlider.textFromValueFunction = [](double v) {
        int pct = static_cast<int>(std::round(v * 100.0));
        if (pct == 0) return juce::String("C");
        if (pct < 0)  return juce::String(-pct) + "L";
        return juce::String(pct) + "R";
    };
    addAndMakeVisible(panSlider);
    panAtt = std::make_unique<SliderAttachment>(apvts, prefix + "pan", panSlider);

    tuneSlider.setName("SlotTune");
    tuneSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    tuneSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 44, 14);
    tuneSlider.setTextValueSuffix(" st");
    addAndMakeVisible(tuneSlider);
    tuneAtt = std::make_unique<SliderAttachment>(apvts, prefix + "tune", tuneSlider);

    // 6. Solo, Mute, Phase Buttons
    soloButton.setClickingTogglesState(true);
    soloButton.setTooltip("Solo");
    addAndMakeVisible(soloButton);
    soloAtt = std::make_unique<ButtonAttachment>(apvts, prefix + "solo", soloButton);

    muteButton.setClickingTogglesState(true);
    muteButton.setTooltip("Mute");
    addAndMakeVisible(muteButton);
    muteAtt = std::make_unique<ButtonAttachment>(apvts, prefix + "mute", muteButton);

    phaseButton.setClickingTogglesState(true);
    phaseButton.setTooltip("Invert Phase");
    addAndMakeVisible(phaseButton);
    phaseAtt = std::make_unique<ButtonAttachment>(apvts, prefix + "phase", phaseButton);

    // 7. Envelope mini sliders
    attackSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    attackSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    attackSlider.setTooltip("Attack Time (0.1 - 50 ms)");
    addAndMakeVisible(attackSlider);
    attackAtt = std::make_unique<SliderAttachment>(apvts, prefix + "attack", attackSlider);

    decaySlider.setSliderStyle(juce::Slider::LinearHorizontal);
    decaySlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    decaySlider.setTooltip("Decay Time (10 - 2000 ms)");
    addAndMakeVisible(decaySlider);
    decayAtt = std::make_unique<SliderAttachment>(apvts, prefix + "decay", decaySlider);

    // 8. Velocity Range Sliders (Min and Max Velocity)
    minVelSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    minVelSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    minVelSlider.setTooltip("Min Velocity Limit");
    addAndMakeVisible(minVelSlider);
    minVelAtt = std::make_unique<SliderAttachment>(apvts, prefix + "minVel", minVelSlider);

    maxVelSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    maxVelSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    maxVelSlider.setTooltip("Max Velocity Limit");
    addAndMakeVisible(maxVelSlider);
    maxVelAtt = std::make_unique<SliderAttachment>(apvts, prefix + "maxVel", maxVelSlider);

    // Labels
    volLabel.setText("LEVEL", juce::dontSendNotification);
    volLabel.setFont(juce::Font(8.5f, juce::Font::bold));
    volLabel.setColour(juce::Label::textColourId, SlateLookAndFeel::textDim);
    volLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(volLabel);

    panLabel.setText("PAN", juce::dontSendNotification);
    panLabel.setFont(juce::Font(8.5f, juce::Font::bold));
    panLabel.setColour(juce::Label::textColourId, SlateLookAndFeel::textDim);
    panLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(panLabel);

    tuneLabel.setText("TUNE", juce::dontSendNotification);
    tuneLabel.setFont(juce::Font(8.5f, juce::Font::bold));
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
    float p = slot.getCurrentPeakMeter();
    currentPeak = p;
    if (p > peakHold)
    {
        peakHold = p;
        peakHoldTimer = 25; // Hold for 25 frames (~700ms)
    }
    else
    {
        if (peakHoldTimer > 0)
            --peakHoldTimer;
        else
            peakHold = std::max(0.0f, peakHold - 0.04f);
    }

    if (nameLabel.getText() != slot.getSampleName() && !slot.getSampleName().isEmpty())
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

    // 1. Strip chassis background with brushed dark aluminum gradient
    juce::ColourGradient stripGrad(juce::Colour(0xff181b22), 0.0f, 0.0f,
                                   juce::Colour(0xff0e1015), 0.0f, static_cast<float>(h), false);
    g.setGradientFill(stripGrad);
    g.fillRoundedRectangle(2.0f, 2.0f, static_cast<float>(w) - 4.0f, static_cast<float>(h) - 4.0f, 4.0f);

    // Subtle metallic border
    g.setColour(SlateLookAndFeel::slateBorder);
    g.drawRoundedRectangle(2.0f, 2.0f, static_cast<float>(w) - 4.0f, static_cast<float>(h) - 4.0f, 4.0f, 1.0f);

    // 2. Channel Identification Header Bar
    juce::Rectangle<float> headerBar(3.0f, 3.0f, static_cast<float>(w) - 6.0f, 22.0f);
    g.setColour(juce::Colour(0xff1c212a));
    g.fillRoundedRectangle(headerBar, 3.0f);

    // Colored Accent Tab on Left
    juce::Rectangle<float> colorTab(headerBar.getX(), headerBar.getY(), 6.0f, headerBar.getHeight());
    g.setColour(slotThemeColor);
    g.fillRoundedRectangle(colorTab, 2.0f);

    // Channel Title
    g.setColour(SlateLookAndFeel::textLight);
    g.setFont(juce::Font(10.0f, juce::Font::bold));
    g.drawText("CH " + juce::String(slotIndex + 1), 32, 3, 50, 22, juce::Justification::left);

    // Channel Enable Jewel Indicator LED
    bool isEnabled = enableButton.getToggleState();
    float ledSize = 7.0f;
    float ledX = 18.0f;
    float ledY = headerBar.getCentreY() - ledSize * 0.5f;

    if (isEnabled)
    {
        // Glowing LED
        g.setColour(slotThemeColor.withAlpha(0.45f));
        g.fillEllipse(ledX - 2.0f, ledY - 2.0f, ledSize + 4.0f, ledSize + 4.0f);
        g.setColour(slotThemeColor);
        g.fillEllipse(ledX, ledY, ledSize, ledSize);
        g.setColour(juce::Colours::white);
        g.fillEllipse(ledX + 1.5f, ledY + 1.5f, 2.0f, 2.0f);
    }
    else
    {
        // Dark unlit LED
        g.setColour(juce::Colour(0xff101318));
        g.fillEllipse(ledX, ledY, ledSize, ledSize);
        g.setColour(SlateLookAndFeel::slateBorder);
        g.drawEllipse(ledX, ledY, ledSize, ledSize, 0.8f);
    }

    // 3. Recessed LCD Sample Name Display Box
    juce::Rectangle<float> nameBox(6.0f, 26.0f, static_cast<float>(w) - 12.0f, 18.0f);
    g.setColour(juce::Colour(0xff090c10));
    g.fillRoundedRectangle(nameBox, 3.0f);
    g.setColour(SlateLookAndFeel::slateBorder);
    g.drawRoundedRectangle(nameBox, 3.0f, 1.0f);

    // 4. Waveform Thumbnail Display Box
    juce::Rectangle<float> waveBox(6.0f, 48.0f, static_cast<float>(w) - 12.0f, 32.0f);
    g.setColour(juce::Colour(0xff0a0d12));
    g.fillRoundedRectangle(waveBox, 3.0f);
    g.setColour(SlateLookAndFeel::slateBorder);
    g.drawRoundedRectangle(waveBox, 3.0f, 1.0f);

    // Center baseline in waveform box
    g.setColour(juce::Colour(0xff141922));
    g.drawHorizontalLine(static_cast<int>(waveBox.getCentreY()), waveBox.getX() + 2.0f, waveBox.getRight() - 2.0f);

    // Render waveform thumbnail
    if (!waveformThumbnail.empty())
    {
        const int numPts = static_cast<int>(waveformThumbnail.size());
        const float step = (waveBox.getWidth() - 4.0f) / static_cast<float>(numPts);
        const float midY = waveBox.getCentreY();

        for (int i = 0; i < numPts; ++i)
        {
            float peakH = waveformThumbnail[static_cast<size_t>(i)] * (waveBox.getHeight() * 0.44f);
            float px = waveBox.getX() + 2.0f + static_cast<float>(i) * step;

            // Halo glow
            g.setColour(slotThemeColor.withAlpha(0.35f));
            g.drawVerticalLine(static_cast<int>(px), midY - peakH - 1.0f, midY + peakH + 1.0f);

            // Core line
            g.setColour(juce::Colours::white);
            g.drawVerticalLine(static_cast<int>(px), midY - peakH, midY + peakH);
        }

        // Active hit flash playhead across waveform when sample triggers
        if (currentPeak > 0.05f)
        {
            g.setColour(juce::Colours::white.withAlpha(std::min(1.0f, currentPeak)));
            g.fillRect(waveBox.getX() + 3.0f, waveBox.getY() + 2.0f, 2.5f, waveBox.getHeight() - 4.0f);
        }
    }

    // 5. 12-Segment Studio LED Ladder Peak Meter beside volume fader
    const float meterX = 52.0f;
    const float meterY = 114.0f;
    const float meterW = 7.0f;
    const int numLeds = 12;
    const float ledH = 7.0f;
    const float ledSpacing = 1.3f;

    float normPeak = juce::jlimit(0.0f, 1.0f, currentPeak);
    int litLeds = static_cast<int>(normPeak * static_cast<float>(numLeds));
    int holdLed = static_cast<int>(juce::jlimit(0.0f, 1.0f, peakHold) * static_cast<float>(numLeds));

    for (int l = 0; l < numLeds; ++l)
    {
        float ly = meterY + static_cast<float>(numLeds - 1 - l) * (ledH + ledSpacing);
        bool isLit = (l < litLeds);
        bool isHold = (l == holdLed && holdLed > 0);

        juce::Colour ledColor;
        if (l >= 10)      // Red clip LEDs
            ledColor = SlateLookAndFeel::muteRed;
        else if (l >= 8)  // Amber warning LEDs
            ledColor = SlateLookAndFeel::amberGlow;
        else              // Green nominal LEDs
            ledColor = juce::Colour(0xff00e676);

        if (isLit || isHold)
        {
            // Lit LED glow
            g.setColour(ledColor.withAlpha(0.35f));
            g.fillRoundedRectangle(meterX - 1.0f, ly - 1.0f, meterW + 2.0f, ledH + 2.0f, 1.5f);
            g.setColour(ledColor);
            g.fillRoundedRectangle(meterX, ly, meterW, ledH, 1.0f);
        }
        else
        {
            // Dark unlit translucent LED lens
            g.setColour(ledColor.withAlpha(0.12f));
            g.fillRoundedRectangle(meterX, ly, meterW, ledH, 1.0f);
            g.setColour(juce::Colour(0xff090b0e));
            g.drawRoundedRectangle(meterX, ly, meterW, ledH, 1.0f, 0.6f);
        }
    }

    // 6. Section Dividers & Parameter Titles
    g.setFont(juce::Font(8.0f, juce::Font::bold));
    g.setColour(SlateLookAndFeel::textDim);
    g.drawText("ATT / DEC", 10, h - 56, w - 20, 10, juce::Justification::centred);
    g.drawText("VELOCITY RANGE", 10, h - 28, w - 20, 10, juce::Justification::centred);

    // 7. Flash hit pip when sample triggers
    if (currentPeak > 0.05f)
    {
        float hitX = static_cast<float>(w) * 0.5f;
        g.setColour(slotThemeColor.withAlpha(0.7f));
        g.fillEllipse(hitX - 4.0f, static_cast<float>(h) - 15.0f, 8.0f, 8.0f);
        g.setColour(juce::Colours::white);
        g.fillEllipse(hitX - 2.0f, static_cast<float>(h) - 13.0f, 4.0f, 4.0f);
    }

    // 8. Drag-and-drop hover glow overlay
    if (isDragHovered)
    {
        g.setColour(slotThemeColor.withAlpha(0.28f));
        g.fillRoundedRectangle(2.0f, 2.0f, static_cast<float>(w) - 4.0f, static_cast<float>(h) - 4.0f, 4.0f);
        g.setColour(slotThemeColor);
        g.drawRoundedRectangle(2.0f, 2.0f, static_cast<float>(w) - 4.0f, static_cast<float>(h) - 4.0f, 4.0f, 2.0f);

        g.setFont(juce::Font(12.5f, juce::Font::bold));
        g.setColour(juce::Colours::white);
        g.drawText("DROP TO LOAD", 0, h / 2 - 12, w, 24, juce::Justification::centred);
    }
}

void SingleSlotStrip::resized()
{
    const int w = getWidth();
    const int h = getHeight();

    // Enable button covers the top-left corner indicator
    enableButton.setBounds(6, 4, 20, 20);
    nameLabel.setBounds(6, 26, w - 12, 18);

    // Row of Action buttons
    loadButton.setBounds(6, 84, 46, 20);
    factoryButton.setBounds(56, 84, 52, 20);
    testHitButton.setBounds(112, 84, 38, 20);

    // Fader & Meter
    volLabel.setBounds(10, 106, 38, 12);
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

    // Envelopes: Attack & Decay
    attackSlider.setBounds(8, h - 44, (w - 18) / 2, 12);
    decaySlider.setBounds(10 + (w - 18) / 2, h - 44, (w - 18) / 2, 12);

    // Velocity Range Sliders
    minVelSlider.setBounds(8, h - 16, (w - 18) / 2, 12);
    maxVelSlider.setBounds(10 + (w - 18) / 2, h - 16, (w - 18) / 2, 12);
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
    const int numStrips = static_cast<int>(slotStrips.size());
    if (numStrips == 0) return;

    const int stripW = w / numStrips;
    for (int i = 0; i < numStrips; ++i)
    {
        slotStrips[static_cast<size_t>(i)]->setBounds(i * stripW, 0, stripW, h);
    }
}

} // namespace yeahwaah
