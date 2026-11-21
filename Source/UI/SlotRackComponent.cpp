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

    // 1. SSL Channel ON/OFF Backlit Pushbutton (NO checkboxes!)
    enableButton.setClickingTogglesState(true);
    enableButton.setTooltip("Channel On/Off");
    enableButton.onClick = [this] { updateEnableButtonText(); };
    addAndMakeVisible(enableButton);
    enableAtt = std::make_unique<ButtonAttachment>(apvts, prefix + "enable", enableButton);
    updateEnableButtonText();

    // 2. Name label (Recessed LCD Display)
    nameLabel.setText(slot.getSampleName().isEmpty() ? "EMPTY" : slot.getSampleName(), juce::dontSendNotification);
    nameLabel.setFont(juce::Font(11.5f, juce::Font::bold));
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

    // 4. Large SSL Volume Fader (Console Vertical Slider)
    volumeSlider.setName("SlotVolume");
    volumeSlider.setSliderStyle(juce::Slider::LinearVertical);
    volumeSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 16);
    addAndMakeVisible(volumeSlider);
    volumeAtt = std::make_unique<SliderAttachment>(apvts, prefix + "volume", volumeSlider);

    // 5. Large SSL Pan & Tune Knobs
    panSlider.setName("SlotPan");
    panSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    panSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 48, 15);
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
    tuneSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 48, 15);
    addAndMakeVisible(tuneSlider);
    tuneAtt = std::make_unique<SliderAttachment>(apvts, prefix + "tune", tuneSlider);

    // 6. SSL Backlit Pushbuttons: Solo, Mute, Phase
    soloButton.setClickingTogglesState(true);
    soloButton.setTooltip("Solo");
    addAndMakeVisible(soloButton);
    soloAtt = std::make_unique<ButtonAttachment>(apvts, prefix + "solo", soloButton);

    muteButton.setClickingTogglesState(true);
    muteButton.setTooltip("Mute");
    addAndMakeVisible(muteButton);
    muteAtt = std::make_unique<ButtonAttachment>(apvts, prefix + "mute", muteButton);

    phaseButton.setClickingTogglesState(true);
    phaseButton.setTooltip("Invert Phase (ø)");
    addAndMakeVisible(phaseButton);
    phaseAtt = std::make_unique<ButtonAttachment>(apvts, prefix + "phase", phaseButton);

    // 7. Envelope Sliders: Attack & Decay
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

    // 8. Velocity Limit Sliders (Min and Max Velocity)
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

    // SSL Silkscreen Labels
    panLabel.setText("PAN", juce::dontSendNotification);
    panLabel.setFont(juce::Font(9.5f, juce::Font::bold));
    panLabel.setColour(juce::Label::textColourId, SlateLookAndFeel::textDim);
    panLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(panLabel);

    tuneLabel.setText("TUNE", juce::dontSendNotification);
    tuneLabel.setFont(juce::Font(9.5f, juce::Font::bold));
    tuneLabel.setColour(juce::Label::textColourId, SlateLookAndFeel::textDim);
    tuneLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(tuneLabel);

    attLabel.setText("ATT", juce::dontSendNotification);
    attLabel.setFont(juce::Font(8.5f, juce::Font::bold));
    attLabel.setColour(juce::Label::textColourId, SlateLookAndFeel::textDim);
    attLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(attLabel);

    decLabel.setText("DEC", juce::dontSendNotification);
    decLabel.setFont(juce::Font(8.5f, juce::Font::bold));
    decLabel.setColour(juce::Label::textColourId, SlateLookAndFeel::textDim);
    decLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(decLabel);

    minVelLabel.setText("MIN", juce::dontSendNotification);
    minVelLabel.setFont(juce::Font(8.5f, juce::Font::bold));
    minVelLabel.setColour(juce::Label::textColourId, SlateLookAndFeel::textDim);
    minVelLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(minVelLabel);

    maxVelLabel.setText("MAX", juce::dontSendNotification);
    maxVelLabel.setFont(juce::Font(8.5f, juce::Font::bold));
    maxVelLabel.setColour(juce::Label::textColourId, SlateLookAndFeel::textDim);
    maxVelLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(maxVelLabel);

    slot.getWaveformThumbnail(waveformThumbnail, 48);
}

void SingleSlotStrip::updateEnableButtonText()
{
    bool isOn = enableButton.getToggleState();
    enableButton.setButtonText(isOn ? "CH ON" : "CH OFF");
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
    updateEnableButtonText();

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

    // 1. SSL Console Channel Chassis Fill (Brushed Gunmetal / Dark Slate)
    juce::ColourGradient stripGrad(juce::Colour(0xff1a1e27), 0.0f, 0.0f,
                                   juce::Colour(0xff0f1117), 0.0f, static_cast<float>(h), false);
    g.setGradientFill(stripGrad);
    g.fillRoundedRectangle(2.0f, 2.0f, static_cast<float>(w) - 4.0f, static_cast<float>(h) - 4.0f, 4.0f);

    // Beveled metallic border
    g.setColour(SlateLookAndFeel::slateBorder);
    g.drawRoundedRectangle(2.0f, 2.0f, static_cast<float>(w) - 4.0f, static_cast<float>(h) - 4.0f, 4.0f, 1.0f);

    // Hairline separation line on right
    g.setColour(juce::Colour(0xff07080b));
    g.drawVerticalLine(w - 1, 0.0f, static_cast<float>(h));

    // 2. Channel Identification Header Bar (SSL Style Channel Header)
    juce::Rectangle<float> headerBar(4.0f, 4.0f, static_cast<float>(w) - 8.0f, 26.0f);
    g.setColour(juce::Colour(0xff222734));
    g.fillRoundedRectangle(headerBar, 3.0f);

    // Colored SSL Accent Badge on Left
    juce::Rectangle<float> colorTab(headerBar.getX(), headerBar.getY(), 6.0f, headerBar.getHeight());
    g.setColour(slotThemeColor);
    g.fillRoundedRectangle(colorTab, 2.0f);

    // Channel Title in Bold Silkscreen
    g.setColour(SlateLookAndFeel::textLight);
    g.setFont(juce::Font(11.0f, juce::Font::bold));
    g.drawText("CH " + juce::String(slotIndex + 1), 16, 4, 60, 26, juce::Justification::centredLeft);

    // 3. Recessed LCD Sample Name Display Box
    juce::Rectangle<float> nameBox(6.0f, 34.0f, static_cast<float>(w) - 12.0f, 20.0f);
    g.setColour(juce::Colour(0xff090c10));
    g.fillRoundedRectangle(nameBox, 3.0f);
    g.setColour(SlateLookAndFeel::slateBorder);
    g.drawRoundedRectangle(nameBox, 3.0f, 1.0f);

    // 4. Waveform Thumbnail Display Box
    juce::Rectangle<float> waveBox(6.0f, 58.0f, static_cast<float>(w) - 12.0f, 36.0f);
    g.setColour(juce::Colour(0xff0a0d13));
    g.fillRoundedRectangle(waveBox, 3.0f);
    g.setColour(SlateLookAndFeel::slateBorder);
    g.drawRoundedRectangle(waveBox, 3.0f, 1.0f);

    // Center grid line in waveform box
    g.setColour(juce::Colour(0xff161c28));
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
            g.setColour(slotThemeColor.withAlpha(0.40f));
            g.drawVerticalLine(static_cast<int>(px), midY - peakH - 1.0f, midY + peakH + 1.0f);

            // Core white line
            g.setColour(juce::Colours::white);
            g.drawVerticalLine(static_cast<int>(px), midY - peakH, midY + peakH);
        }

        // Active hit flash playhead across waveform when sample triggers
        if (currentPeak > 0.04f)
        {
            g.setColour(juce::Colours::white.withAlpha(std::min(1.0f, currentPeak * 1.2f)));
            g.fillRect(waveBox.getX() + 3.0f, waveBox.getY() + 2.0f, 3.0f, waveBox.getHeight() - 4.0f);
        }
    }

    // 5. SSL Calibrated dB Scale Markings beside Volume Fader
    const float faderX = 8.0f;
    const float faderY = 126.0f;
    const float meterX = faderX + 66.0f;
    const float meterW = 9.0f;

    // Printed dB values beside fader
    g.setFont(juce::Font(8.0f, juce::Font::bold));
    g.setColour(SlateLookAndFeel::textDim);
    g.drawText("+12", 2, static_cast<int>(faderY) + 4, 16, 10, juce::Justification::centredRight);
    g.drawText("0", 6, static_cast<int>(faderY) + 50, 12, 10, juce::Justification::centredRight);
    g.drawText("-12", 2, static_cast<int>(faderY) + 96, 16, 10, juce::Justification::centredRight);
    g.drawText("-inf", 2, static_cast<int>(faderY) + 152, 16, 10, juce::Justification::centredRight);

    // 6. 18-Segment Studio LED Ladder Peak Meter
    const int numLeds = 18;
    const float ledH = 6.4f;
    const float ledSpacing = 1.6f;

    float normPeak = juce::jlimit(0.0f, 1.0f, currentPeak);
    int litLeds = static_cast<int>(normPeak * static_cast<float>(numLeds));
    int holdLed = static_cast<int>(juce::jlimit(0.0f, 1.0f, peakHold) * static_cast<float>(numLeds));

    for (int l = 0; l < numLeds; ++l)
    {
        float ly = faderY + 6.0f + static_cast<float>(numLeds - 1 - l) * (ledH + ledSpacing);
        bool isLit = (l < litLeds);
        bool isHold = (l == holdLed && holdLed > 0);

        juce::Colour ledColor;
        if (l >= 15)      // Red clip LEDs
            ledColor = SlateLookAndFeel::muteRed;
        else if (l >= 12)  // Amber warning LEDs
            ledColor = SlateLookAndFeel::amberGlow;
        else              // Green nominal LEDs
            ledColor = juce::Colour(0xff00e676);

        if (isLit || isHold)
        {
            // Lit LED glow
            g.setColour(ledColor.withAlpha(0.38f));
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

    // 7. Section Titles: Envelope & Velocity Shapers
    g.setFont(juce::Font(8.5f, juce::Font::bold));
    g.setColour(SlateLookAndFeel::textDim);
    g.drawText("ENVELOPE SHAPER", 8, h - 90, w - 16, 12, juce::Justification::centred);
    g.drawText("VELOCITY RANGE", 8, h - 45, w - 16, 12, juce::Justification::centred);

    // 8. Flash Hit Jewel when sample triggers
    if (currentPeak > 0.04f)
    {
        float hitX = static_cast<float>(w) * 0.5f;
        g.setColour(slotThemeColor.withAlpha(0.85f));
        g.fillEllipse(hitX - 5.0f, static_cast<float>(h) - 10.0f, 10.0f, 10.0f);
        g.setColour(juce::Colours::white);
        g.fillEllipse(hitX - 2.5f, static_cast<float>(h) - 7.5f, 5.0f, 5.0f);
    }

    // 9. Drag-and-drop hover glow overlay
    if (isDragHovered)
    {
        g.setColour(slotThemeColor.withAlpha(0.30f));
        g.fillRoundedRectangle(2.0f, 2.0f, static_cast<float>(w) - 4.0f, static_cast<float>(h) - 4.0f, 4.0f);
        g.setColour(slotThemeColor);
        g.drawRoundedRectangle(2.0f, 2.0f, static_cast<float>(w) - 4.0f, static_cast<float>(h) - 4.0f, 4.0f, 2.0f);

        g.setFont(juce::Font(13.0f, juce::Font::bold));
        g.setColour(juce::Colours::white);
        g.drawText("DROP TO LOAD", 0, h / 2 - 12, w, 24, juce::Justification::centred);
    }
}

void SingleSlotStrip::resized()
{
    const int w = getWidth();
    const int h = getHeight();

    // 1. Header: Channel on/off backlit button
    enableButton.setBounds(w - 74, 5, 68, 24);
    nameLabel.setBounds(6, 34, w - 12, 20);

    // 2. Row of Action buttons
    const int btnY = 98;
    const int btnH = 22;
    int actionW = (w - 18) / 3;
    loadButton.setBounds(6, btnY, actionW, btnH);
    factoryButton.setBounds(8 + actionW, btnY, actionW + 6, btnH);
    testHitButton.setBounds(16 + actionW * 2, btnY, w - (18 + actionW * 2), btnH);

    // 3. Left Side: Large Volume Fader (Console Vertical Slider)
    volumeSlider.setBounds(18, 126, 52, 172);

    // 4. Right Side: Pan & Tune Big Knobs
    const int rightX = 84;
    const int knobW = (w - rightX - 8) / 2;
    panLabel.setBounds(rightX, 122, knobW, 14);
    panSlider.setBounds(rightX, 136, knobW, 66);

    tuneLabel.setBounds(rightX + knobW + 2, 122, knobW, 14);
    tuneSlider.setBounds(rightX + knobW + 2, 136, knobW, 66);

    // 5. SSL Illuminated Pushbuttons: Solo, Mute, Phase
    const int btnRowY = 218;
    const int sH = 28;
    int sW = (w - rightX - 12) / 3;
    soloButton.setBounds(rightX, btnRowY, sW, sH);
    muteButton.setBounds(rightX + sW + 4, btnRowY, sW, sH);
    phaseButton.setBounds(rightX + (sW + 4) * 2, btnRowY, sW, sH);

    // 6. Envelope Section: Attack & Decay
    const int envY = h - 80;
    const int envH = 22;
    const int envHalfW = (w - 24) / 2;

    attLabel.setBounds(8, envY, 26, envH);
    attackSlider.setBounds(34, envY, envHalfW - 24, envH);

    decLabel.setBounds(12 + envHalfW, envY, 26, envH);
    decaySlider.setBounds(38 + envHalfW, envY, envHalfW - 24, envH);

    // 7. Velocity Limit Sliders: Min & Max
    const int velY = h - 36;
    minVelLabel.setBounds(8, velY, 26, envH);
    minVelSlider.setBounds(34, velY, envHalfW - 24, envH);

    maxVelLabel.setBounds(12 + envHalfW, velY, 26, envH);
    maxVelSlider.setBounds(38 + envHalfW, velY, envHalfW - 24, envH);
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
