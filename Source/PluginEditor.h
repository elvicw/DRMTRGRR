#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"
#include "UI/SlateLookAndFeel.h"
#include "UI/HeaderBar.h"
#include "UI/TriggerVisualizer.h"
#include "UI/DetectorPanel.h"
#include "UI/SlotRackComponent.h"

namespace yeahwaah
{

class DrmtrgrrAudioProcessorEditor : public juce::AudioProcessorEditor,
                                    public juce::Timer
{
public:
    explicit DrmtrgrrAudioProcessorEditor(DrmtrgrrAudioProcessor&);
    ~DrmtrgrrAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;

private:
    DrmtrgrrAudioProcessor& audioProcessor;
    SlateLookAndFeel slateLookAndFeel;

    HeaderBar headerBar;
    TriggerVisualizer visualizer;
    DetectorPanel detectorPanel;
    SlotRackComponent slotRack;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DrmtrgrrAudioProcessorEditor)
};

} // namespace yeahwaah
