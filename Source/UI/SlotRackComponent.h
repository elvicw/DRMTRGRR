#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "../DSP/SampleSlot.h"
#include "HardwareControls.h"

namespace yeahwaah
{

class DrmtrgrrAudioProcessor;

class SingleSlotStrip : public juce::Component,
                        public juce::FileDragAndDropTarget
{
public:
    static constexpr int kWidth = 219;
    static constexpr int kHeight = 298;

    SingleSlotStrip(int slotIndex, DrmtrgrrAudioProcessor& processor);
    ~SingleSlotStrip() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent& e) override;

    bool isInterestedInFileDrag(const juce::StringArray& files) override;
    void fileDragEnter(const juce::StringArray& files, int x, int y) override;
    void fileDragExit(const juce::StringArray& files) override;
    void filesDropped(const juce::StringArray& files, int x, int y) override;

    void updateMeterAndWaveform();

private:
    const int slotIndex;
    DrmtrgrrAudioProcessor& processor;
    SampleSlot& slot;
    bool isDragHovered = false;
    juce::Colour theme;

    juce::TextButton menuButton { juce::CharPointer_UTF8("\xe2\x8b\xaf") };
    hw::HardwareKnob volumeKnob, panKnob, tuneKnob;
    hw::HardwareButton muteButton { "M", hw::HardwareButton::Style::Key };
    hw::HardwareButton soloButton { "S", hw::HardwareButton::Style::Key };
    hw::HardwareButton phaseButton { juce::CharPointer_UTF8("\xc3\x98"), hw::HardwareButton::Style::Key };

    std::unique_ptr<juce::FileChooser> fileChooser;
    std::vector<float> waveformThumbnail;
    juce::String shownName;
    float currentPeak = 0.0f;
    int flash = 0;

    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;
    std::unique_ptr<ButtonAttachment> muteAtt, soloAtt, phaseAtt;

    juce::Rectangle<int> waveBox() const { return { 0, 43, kWidth, 59 }; }
    void showMenu();
    void showAdvanced();
    void chooseSampleFile();
    void cycleFactoryDrum();
    void refreshWaveform();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SingleSlotStrip)
};

class SlotRackComponent : public juce::Component
{
public:
    static constexpr int kOriginY = 620;

    explicit SlotRackComponent(DrmtrgrrAudioProcessor& processor);
    ~SlotRackComponent() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void updateMeters();

private:
    std::vector<std::unique_ptr<SingleSlotStrip>> slotStrips;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SlotRackComponent)
};

} // namespace yeahwaah
