#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "../DSP/SampleSlot.h"

namespace yeahwaah
{

class DrmtrgrrAudioProcessor;

class SingleSlotStrip : public juce::Component,
                        public juce::FileDragAndDropTarget
{
public:
    SingleSlotStrip(int slotIndex, DrmtrgrrAudioProcessor& processor);
    ~SingleSlotStrip() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;

    // File Drag and Drop
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
    juce::Colour slotThemeColor;

    // Controls
    juce::ToggleButton enableButton;
    juce::Label nameLabel;
    juce::TextButton loadButton { "LOAD" };
    juce::TextButton factoryButton { "FACTORY" };
    juce::TextButton testHitButton { "HIT" };

    juce::Slider volumeSlider;
    juce::Slider panSlider;
    juce::Slider tuneSlider;

    juce::ToggleButton soloButton { "S" };
    juce::ToggleButton muteButton { "M" };
    juce::ToggleButton phaseButton { "ø" };

    juce::Slider attackSlider;
    juce::Slider decaySlider;
    juce::Slider minVelSlider;
    juce::Slider maxVelSlider;

    juce::Label volLabel, panLabel, tuneLabel;

    std::unique_ptr<juce::FileChooser> fileChooser;
    std::vector<float> waveformThumbnail;
    float currentPeak = 0.0f;
    float peakHold = 0.0f;
    int peakHoldTimer = 0;

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;

    std::unique_ptr<ButtonAttachment> enableAtt;
    std::unique_ptr<SliderAttachment> volumeAtt;
    std::unique_ptr<SliderAttachment> panAtt;
    std::unique_ptr<SliderAttachment> tuneAtt;
    std::unique_ptr<ButtonAttachment> soloAtt;
    std::unique_ptr<ButtonAttachment> muteAtt;
    std::unique_ptr<ButtonAttachment> phaseAtt;
    std::unique_ptr<SliderAttachment> attackAtt;
    std::unique_ptr<SliderAttachment> decayAtt;
    std::unique_ptr<SliderAttachment> minVelAtt;
    std::unique_ptr<SliderAttachment> maxVelAtt;

    void chooseSampleFile();
    void cycleFactoryDrum();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SingleSlotStrip)
};

class SlotRackComponent : public juce::Component
{
public:
    SlotRackComponent(DrmtrgrrAudioProcessor& processor);
    ~SlotRackComponent() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void updateMeters();

private:
    std::vector<std::unique_ptr<SingleSlotStrip>> slotStrips;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SlotRackComponent)
};

} // namespace yeahwaah
