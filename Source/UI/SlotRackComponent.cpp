#include "SlotRackComponent.h"
#include "SlateLookAndFeel.h"
#include "../PluginProcessor.h"
#include <cmath>

namespace yeahwaah
{

namespace
{
const char* const kSlotNames[] = { "KICK", "SNARE", "RIM", "ROOM", "WIRES", "AUX" };

bool isAudioFile(const juce::String& f)
{
    return f.endsWithIgnoreCase(".wav") || f.endsWithIgnoreCase(".aif") || f.endsWithIgnoreCase(".aiff")
        || f.endsWithIgnoreCase(".mp3") || f.endsWithIgnoreCase(".flac");
}
} // namespace

SingleSlotStrip::SingleSlotStrip(int index, DrmtrgrrAudioProcessor& proc)
    : slotIndex(index), processor(proc), slot(proc.getSlot(index)),
      theme(SlateLookAndFeel::getSlotColor(index)),
      volumeKnob(proc.getAPVTS(), "slot_" + juce::String(index) + "_volume", "VOL", " dB", 1, nullptr, 14.0f),
      panKnob(proc.getAPVTS(), "slot_" + juce::String(index) + "_pan", "PAN", {}, 2,
               [](double v) {
                   if (std::abs(v) < 0.005) return juce::String("C");
                   return juce::String(juce::roundToInt(std::abs(v) * 100.0)) + (v < 0 ? " L" : " R");
               },
               14.0f),
      tuneKnob(proc.getAPVTS(), "slot_" + juce::String(index) + "_tune", "TUNE", " st", 1, nullptr, 14.0f)
{
    const juce::String p = "slot_" + juce::String(index) + "_";
    auto& apvts = proc.getAPVTS();

    menuButton.onClick = [this] { showMenu(); };
    menuButton.setTooltip("Sample options");
    addAndMakeVisible(menuButton);

    addAndMakeVisible(volumeKnob);
    addAndMakeVisible(panKnob);
    addAndMakeVisible(tuneKnob);
    addAndMakeVisible(muteButton);
    addAndMakeVisible(soloButton);
    addAndMakeVisible(phaseButton);
    muteButton.setTooltip("Mute");
    soloButton.setTooltip("Solo");
    phaseButton.setTooltip("Invert phase");

    muteAtt = std::make_unique<ButtonAttachment>(apvts, p + "mute", muteButton);
    soloAtt = std::make_unique<ButtonAttachment>(apvts, p + "solo", soloButton);
    phaseAtt = std::make_unique<ButtonAttachment>(apvts, p + "phase", phaseButton);

    refreshWaveform();
}

void SingleSlotStrip::refreshWaveform()
{
    shownName = slot.getSampleName();
    slot.getWaveformThumbnail(waveformThumbnail, 64);
}

void SingleSlotStrip::resized()
{
    menuButton.setBounds(kWidth - 34, 14, 26, 24);

    const int kw = 74, ky = 108, kh = 112;
    volumeKnob.setBounds(36 - kw / 2, ky, kw, kh);
    panKnob.setBounds(110 - kw / 2, ky, kw, kh);
    tuneKnob.setBounds(183 - kw / 2, ky, kw, kh);

    const int by = 240, bh = 42, bw = 56;
    muteButton.setBounds(12, by, bw, bh);
    soloButton.setBounds(kWidth / 2 - bw / 2, by, bw, bh);
    phaseButton.setBounds(kWidth - 12 - bw, by, bw, bh);
}

void SingleSlotStrip::paint(juce::Graphics& g)
{
    using L = SlateLookAndFeel;
    auto b = getLocalBounds().toFloat();
    hw::drawPanel(g, b, 6.0f);

    g.setColour(theme);
    g.fillRoundedRectangle(b.getX() + 4, b.getY() + 4, b.getWidth() - 8, 7.0f, 3.0f);

    hw::drawEngravedText(g, juce::String(slotIndex + 1).paddedLeft('0', 2) + " " + kSlotNames[slotIndex],
                         { 10, 14, 110, 24 }, 20.0f, L::textLight, juce::Justification::centredLeft);

    g.setFont(L::boldFont(12.0f));
    g.setColour(L::textDim);
    g.drawText(shownName.isEmpty() ? juce::String("EMPTY") : shownName, 100, 14, kWidth - 140, 24,
               juce::Justification::centredRight, true);

    // Waveform window
    auto wb = waveBox().reduced(8, 0).toFloat();
    g.setColour(juce::Colour(0xff0a0907));
    g.fillRoundedRectangle(wb, 4.0f);
    g.setColour(isDragHovered ? theme : juce::Colour(0xff3a342a));
    g.drawRoundedRectangle(wb.reduced(0.5f), 4.0f, isDragHovered ? 2.0f : 1.0f);

    const float mid = wb.getCentreY();
    g.setColour(theme.withAlpha(0.18f));
    g.drawHorizontalLine((int) mid, wb.getX() + 4, wb.getRight() - 4);

    if (!waveformThumbnail.empty())
    {
        const int n = (int) waveformThumbnail.size();
        const float step = (wb.getWidth() - 8.0f) / (float) n;
        juce::Path wave;
        for (int i = 0; i < n; ++i)
        {
            const float x = wb.getX() + 4.0f + step * (float) i;
            const float amp = juce::jlimit(0.0f, 1.0f, waveformThumbnail[(size_t) i]) * (wb.getHeight() * 0.44f);
            wave.addRectangle(x, mid - amp, juce::jmax(1.0f, step * 0.7f), juce::jmax(1.0f, amp * 2.0f));
        }
        g.setColour(L::getSlotWaveColor(slotIndex));
        g.fillPath(wave);
    }

    if (flash > 0)
    {
        g.setColour(theme.withAlpha(0.25f * (float) flash / 6.0f));
        g.fillRoundedRectangle(wb, 4.0f);
    }
}

void SingleSlotStrip::mouseDown(const juce::MouseEvent& e)
{
    if (waveBox().contains(e.getPosition()))
        showMenu();
}

void SingleSlotStrip::showMenu()
{
    juce::PopupMenu m;
    m.addItem(1, "Load sample...");
    m.addItem(2, "Next factory drum");
    m.addItem(3, "Test hit");
    const bool on = processor.getAPVTS().getRawParameterValue("slot_" + juce::String(slotIndex) + "_enable")->load() > 0.5f;
    m.addItem(4, "Channel enabled", true, on);
    m.addItem(5, "Advanced...");

    m.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(this), [safe = juce::Component::SafePointer<SingleSlotStrip>(this)](int r) {
        if (safe == nullptr)
            return;
        switch (r)
        {
            case 1: safe->chooseSampleFile(); break;
            case 2: safe->cycleFactoryDrum(); break;
            case 3: safe->processor.triggerSlotManually(safe->slotIndex, 1.0f); break;
            case 4:
                if (auto* prm = safe->processor.getAPVTS().getParameter("slot_" + juce::String(safe->slotIndex) + "_enable"))
                {
                    prm->beginChangeGesture();
                    prm->setValueNotifyingHost(prm->getValue() > 0.5f ? 0.0f : 1.0f);
                    prm->endChangeGesture();
                }
                break;
            case 5: safe->showAdvanced(); break;
            default: break;
        }
    });
}

void SingleSlotStrip::showAdvanced()
{
    struct Content : public juce::Component
    {
        juce::Slider sliders[4];
        juce::Label labels[4];
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> atts[4];

        Content(juce::AudioProcessorValueTreeState& apvts, int slotIndex)
        {
            const char* ids[] = { "attack", "decay", "minVel", "maxVel" };
            const char* names[] = { "Attack", "Decay", "Min vel", "Max vel" };
            for (int i = 0; i < 4; ++i)
            {
                sliders[i].setSliderStyle(juce::Slider::LinearHorizontal);
                sliders[i].setTextBoxStyle(juce::Slider::TextBoxRight, false, 60, 22);
                labels[i].setText(names[i], juce::dontSendNotification);
                addAndMakeVisible(sliders[i]);
                addAndMakeVisible(labels[i]);
                atts[i] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
                    apvts, "slot_" + juce::String(slotIndex) + "_" + ids[i], sliders[i]);
            }
            setSize(300, 4 * 30 + 16);
        }

        void resized() override
        {
            auto r = getLocalBounds().reduced(8);
            for (int i = 0; i < 4; ++i)
            {
                auto row = r.removeFromTop(30);
                labels[i].setBounds(row.removeFromLeft(64));
                sliders[i].setBounds(row);
            }
        }
    };

    juce::CallOutBox::launchAsynchronously(std::make_unique<Content>(processor.getAPVTS(), slotIndex),
                                           getScreenBounds().withHeight(60), nullptr);
}

void SingleSlotStrip::chooseSampleFile()
{
    fileChooser = std::make_unique<juce::FileChooser>(
        "Select Drum Sample", juce::File::getSpecialLocation(juce::File::userHomeDirectory),
        "*.wav;*.aif;*.aiff;*.mp3;*.flac");

    auto flags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles;
    fileChooser->launchAsync(flags, [safe = juce::Component::SafePointer<SingleSlotStrip>(this)](const juce::FileChooser& fc) {
        auto result = fc.getResult();
        if (safe != nullptr && result.existsAsFile() && safe->slot.loadFromFile(result, safe->processor.getFormatManager()))
        {
            safe->refreshWaveform();
            safe->repaint();
        }
    });
}

void SingleSlotStrip::cycleFactoryDrum()
{
    static int cycleCounter = 0;
    cycleCounter = (cycleCounter + 1) % static_cast<int>(FactoryDrumType::Count);
    slot.loadFactoryDrum(static_cast<FactoryDrumType>(cycleCounter));
    refreshWaveform();
    repaint();
}

bool SingleSlotStrip::isInterestedInFileDrag(const juce::StringArray& files)
{
    for (const auto& f : files)
        if (isAudioFile(f))
            return true;
    return false;
}

void SingleSlotStrip::fileDragEnter(const juce::StringArray&, int, int)
{
    isDragHovered = true;
    repaint();
}

void SingleSlotStrip::fileDragExit(const juce::StringArray&)
{
    isDragHovered = false;
    repaint();
}

void SingleSlotStrip::filesDropped(const juce::StringArray& files, int, int)
{
    isDragHovered = false;
    if (!files.isEmpty() && slot.loadFromFile(juce::File(files[0]), processor.getFormatManager()))
        refreshWaveform();
    repaint();
}

void SingleSlotStrip::updateMeterAndWaveform()
{
    const float p = slot.getCurrentPeakMeter();
    const bool hit = p > 0.05f && p > currentPeak;
    currentPeak = p;
    if (hit)
        flash = 6;
    else if (flash > 0)
        --flash;
    else if (slot.getSampleName() == shownName)
        return;

    if (slot.getSampleName() != shownName && !slot.getSampleName().isEmpty())
        refreshWaveform();
    repaint();
}

// ---------------------------------------------------------------------------

SlotRackComponent::SlotRackComponent(DrmtrgrrAudioProcessor& processor)
{
    for (int i = 0; i < DrmtrgrrAudioProcessor::NumSlots; ++i)
    {
        slotStrips.push_back(std::make_unique<SingleSlotStrip>(i, processor));
        addAndMakeVisible(*slotStrips.back());
    }
}

void SlotRackComponent::paint(juce::Graphics& g)
{
    hw::drawPanel(g, juce::Rectangle<float>(10.0f, 0.0f, 1480.0f, 380.0f));
    hw::drawEngravedText(g, "SAMPLE RACK", { 10, 8, 1480, 26 }, 17.0f, SlateLookAndFeel::titleAmber);
}

void SlotRackComponent::resized()
{
    for (size_t i = 0; i < slotStrips.size(); ++i)
        slotStrips[i]->setBounds(58 + 240 * (int) i, 670 - kOriginY, SingleSlotStrip::kWidth, SingleSlotStrip::kHeight);
}

void SlotRackComponent::updateMeters()
{
    for (auto& s : slotStrips)
        s->updateMeterAndWaveform();
}

} // namespace yeahwaah
