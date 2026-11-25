#include "DetectorPanel.h"
#include "SlateLookAndFeel.h"

namespace yeahwaah
{

namespace
{
juce::String freqText(double v)
{
    return v >= 1000.0 ? juce::String(v / 1000.0, 1) + " kHz" : juce::String(juce::roundToInt(v)) + " Hz";
}

juce::Rectangle<int> local(int x, int y, int w, int h) { return { x, y - DetectorPanel::kOriginY, w, h }; }
juce::Rectangle<float> localF(float x, float y, float w, float h) { return { x, y - (float) DetectorPanel::kOriginY, w, h }; }
} // namespace

DetectorPanel::DetectorPanel(juce::AudioProcessorValueTreeState& a)
    : apvts(a),
      detailKnob(a, "detail", "DETAIL", " dB", 1),
      sensitivityKnob(a, "sensitivity", "SENSITIVITY", "%", 0),
      retrigPad(a, "retrigger", "RETRIG", " ms", 0),
      dynamicsPad(a, "dynamics", "DYNAMICS", "%", 0),
      trimKnob(a, "inputGain", "TRIM", " dB", 1),
      engineDetailKnob(a, "detail", "DETAIL", " dB", 1),
      engineDynamicsKnob(a, "dynamics", "DYNAMICS", "%", 0),
      lowCutKnob(a, "lowCut", "LO-CUT", {}, 0, freqText),
      highCutKnob(a, "highCut", "HI-CUT", {}, 0, freqText),
      gateThresholdKnob(a, "gateThreshold", "THRESHOLD", " dB", 1),
      gateAttackKnob(a, "gateAttack", "ATTACK", " ms", 1),
      gateHoldKnob(a, "gateHold", "HOLD", " ms", 0),
      gateReleaseKnob(a, "gateRelease", "RELEASE", " ms", 0),
      bleedKnob(a, "leakageAmount", "BLEED", "%", 0)
{
    for (auto* c : std::initializer_list<juce::Component*> {
             &detailKnob, &sensitivityKnob, &retrigPad, &dynamicsPad, &trimKnob, &engineDetailKnob,
             &engineDynamicsKnob, &lowCutKnob, &highCutKnob, &listenButton, &gateEnableButton,
             &gateThresholdKnob, &gateAttackKnob, &gateHoldKnob, &gateReleaseKnob, &bleedEnableButton,
             &bleedKnob, &midiOutButton, &midiSettingsButton, &thresholdBar })
        addAndMakeVisible(c);

    retrigPad.setTooltip("Retrigger lockout time");
    dynamicsPad.setTooltip("Velocity dynamics");
    listenButton.setTooltip("Audition the band-pass filter");
    gateEnableButton.setTooltip("Enable transient gate");
    bleedEnableButton.setTooltip("Enable bleed reduction");
    midiOutButton.setTooltip("Enable MIDI output");
    midiSettingsButton.setTooltip("MIDI note and channel");

    midiSettingsButton.onClick = [this] { showMidiSettings(); };

    thresholdBar.setSliderStyle(juce::Slider::LinearBar);
    thresholdBar.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    thresholdBar.setTooltip("Detection threshold");
    thresholdBarAtt = std::make_unique<SliderAttachment>(apvts, "detail", thresholdBar);

    listenAtt = std::make_unique<ButtonAttachment>(apvts, "auditionFilter", listenButton);
    gateEnableAtt = std::make_unique<ButtonAttachment>(apvts, "gateEnable", gateEnableButton);
    bleedEnableAtt = std::make_unique<ButtonAttachment>(apvts, "leakageEnable", bleedEnableButton);
    midiOutAtt = std::make_unique<ButtonAttachment>(apvts, "midiOutEnable", midiOutButton);
}

void DetectorPanel::setMeters(float inputPeak, float outputPeak)
{
    auto conv = [](float p) {
        return juce::jlimit(0.0f, 1.0f, (juce::Decibels::gainToDecibels(p + 1.0e-5f) + 48.0f) / 48.0f);
    };
    const float i = conv(inputPeak), o = conv(outputPeak);
    if (std::abs(i - inMeter) < 0.01f && std::abs(o - outMeter) < 0.01f)
        return;
    inMeter = i;
    outMeter = o;
    repaint(localF(1370, 150, 110, 250).toNearestInt());
}

void DetectorPanel::setActiveHits(int count)
{
    count = juce::jlimit(0, 3, count);
    if (count == activeHits)
        return;
    activeHits = count;
    repaint(local(1100, 376, 120, 30));
}

void DetectorPanel::paint(juce::Graphics& g)
{
    using L = SlateLookAndFeel;
    auto title = [&](const juce::String& t, juce::Rectangle<int> r) {
        hw::drawEngravedText(g, t, r, 20.0f, L::titleAmber, juce::Justification::centred);
        g.setColour(juce::Colours::black.withAlpha(0.75f));
        g.fillRect(r.getX() + 14, r.getBottom() + 3, r.getWidth() - 28, 2);
        g.setColour(L::titleAmber.withAlpha(0.12f));
        g.fillRect(r.getX() + 14, r.getBottom() + 5, r.getWidth() - 28, 1);
    };

    // Module panels
    hw::drawPanel(g, localF(62, 135, 326, 275));
    hw::drawPanel(g, localF(398, 135, 964, 275));
    hw::drawPanel(g, localF(1370, 135, 110, 275));
    hw::drawPanel(g, localF(10, 422, 405, 190));
    hw::drawPanel(g, localF(426, 422, 297, 190));
    hw::drawPanel(g, localF(738, 422, 498, 190));
    hw::drawPanel(g, localF(1246, 422, 244, 190));

    title("TRIGGER DETECTION", local(62, 140, 326, 28));
    title("TRANSIENT ENGINE", local(10, 428, 405, 28));
    title("BANDPASS FILTER", local(426, 428, 297, 28));
    title("TRANSIENT GATE", local(758, 428, 458, 28));
    title("BLEED & MIDI", local(1246, 428, 244, 28));
    hw::drawEngravedText(g, "LEVEL", local(1370, 142, 110, 24), 15.0f, L::titleAmber);
    hw::drawEngravedText(g, "OSCILLOSCOPE", local(420, 140, 902, 28), 20.0f, L::titleAmber);
    hw::drawEngravedText(g, "LISTEN", local(664, 548, 64, 16), 11.0f, L::textLight);
    hw::drawEngravedText(g, "MIDI OUT", local(1392, 478, 96, 20), 15.0f, L::textLight);

    // Oscilloscope bezel
    auto scope = getScopeBounds().translated(0, -kOriginY).expanded(10);
    g.setColour(juce::Colour(0xff0a0907));
    g.fillRoundedRectangle(scope.toFloat(), 8.0f);
    g.setColour(juce::Colour(0xff3a342a));
    g.drawRoundedRectangle(scope.toFloat().reduced(0.5f), 8.0f, 1.5f);

    hw::drawEngravedText(g, "THRESHOLD", local(470, 378, 112, 22), 14.0f, L::textLight, juce::Justification::centredRight);
    hw::drawEngravedText(g, "HITS", local(1050, 378, 60, 22), 14.0f, L::textLight, juce::Justification::centredRight);
    for (int i = 0; i < 3; ++i)
    {
        auto c = juce::Point<float>(1123.0f + 29.0f * (float) i, 390.0f - (float) kOriginY);
        const bool on = i < activeHits;
        g.setColour(on ? L::titleAmber : juce::Colour(0xff2a2218));
        g.fillEllipse(c.x - 6.0f, c.y - 6.0f, 12.0f, 12.0f);
        if (on)
        {
            g.setColour(L::titleAmber.withAlpha(0.3f));
            g.fillEllipse(c.x - 10.0f, c.y - 10.0f, 20.0f, 20.0f);
        }
    }

    // Level meter columns
    auto meterBox = localF(1384, 172, 82, 200);
    g.setColour(juce::Colour(0xff0a0907));
    g.fillRoundedRectangle(meterBox, 4.0f);
    hw::drawLedBar(g, { meterBox.getX() + 8, meterBox.getY() + 8, 26, meterBox.getHeight() - 16 }, inMeter, 14);
    hw::drawLedBar(g, { meterBox.getRight() - 34, meterBox.getY() + 8, 26, meterBox.getHeight() - 16 }, outMeter, 14);
    hw::drawEngravedText(g, "IN", local(1384, 376, 42, 18), 12.0f, L::textLight);
    hw::drawEngravedText(g, "OUT", local(1424, 376, 42, 18), 12.0f, L::textLight);
}

void DetectorPanel::resized()
{
    // Top row
    detailKnob.setBounds(local(99, 190, 110, 140));
    sensitivityKnob.setBounds(local(245, 190, 110, 140));
    retrigPad.setBounds(local(90, 348, 120, 48));
    dynamicsPad.setBounds(local(228, 348, 132, 48));
    thresholdBar.setBounds(local(593, 380, 422, 20));

    // Middle row
    const int ky = 468, kw = 110, kh = 140;
    trimKnob.setBounds(local(96 - kw / 2, ky, kw, kh));
    engineDetailKnob.setBounds(local(215 - kw / 2, ky, kw, kh));
    engineDynamicsKnob.setBounds(local(332 - kw / 2, ky, kw, kh));
    lowCutKnob.setBounds(local(506 - kw / 2, ky, kw, kh));
    highCutKnob.setBounds(local(633 - kw / 2, ky, kw, kh));
    listenButton.setBounds(local(676, 566, 40, 38));

    gateEnableButton.setBounds(local(764, 432, 20, 20));
    gateThresholdKnob.setBounds(local(817 - kw / 2, ky, kw, kh));
    gateAttackKnob.setBounds(local(937 - kw / 2, ky, kw, kh));
    gateHoldKnob.setBounds(local(1050 - kw / 2, ky, kw, kh));
    gateReleaseKnob.setBounds(local(1164 - kw / 2, ky, kw, kh));

    bleedEnableButton.setBounds(local(1252, 432, 20, 20));
    bleedKnob.setBounds(local(1314 - kw / 2, ky, kw, kh));
    midiOutButton.setBounds(local(1412, 508, 56, 48));
    midiSettingsButton.setBounds(local(1392, 570, 96, 26));
}

void DetectorPanel::showMidiSettings()
{
    struct Content : public juce::Component
    {
        juce::Slider note, channel;
        juce::Label noteLabel { {}, "Note" }, channelLabel { {}, "Channel" };
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> noteAtt, channelAtt;

        explicit Content(juce::AudioProcessorValueTreeState& apvts)
        {
            for (auto* s : { &note, &channel })
            {
                s->setSliderStyle(juce::Slider::LinearHorizontal);
                s->setTextBoxStyle(juce::Slider::TextBoxRight, false, 50, 22);
                addAndMakeVisible(s);
            }
            addAndMakeVisible(noteLabel);
            addAndMakeVisible(channelLabel);
            noteAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "midiNote", note);
            channelAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "midiChannel", channel);
            setSize(280, 80);
        }

        void resized() override
        {
            auto r = getLocalBounds().reduced(8);
            auto row1 = r.removeFromTop(32);
            noteLabel.setBounds(row1.removeFromLeft(64));
            note.setBounds(row1);
            channelLabel.setBounds(r.removeFromLeft(64));
            channel.setBounds(r);
        }
    };

    juce::CallOutBox::launchAsynchronously(std::make_unique<Content>(apvts),
                                           midiSettingsButton.getScreenBounds(), nullptr);
}

} // namespace yeahwaah
