#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"
#include "UI/SlateLookAndFeel.h"
#include "UI/HeaderBar.h"
#include "UI/TriggerVisualizer.h"
#include "UI/DetectorPanel.h"
#include "UI/BrowserView.h"
#include "UI/SettingsView.h"
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

    // Everything is laid out in the 1536x1024 design space and scaled to the window.
    struct Content : public juce::Component
    {
        void paint(juce::Graphics& g) override;
    };
    Content content;
    juce::ComponentBoundsConstrainer constrainer;
    juce::TooltipWindow tooltipWindow { this, 600 };

    HeaderBar headerBar;
    TriggerVisualizer visualizer;
    DetectorPanel detectorPanel;
    BrowserView browserView;
    SettingsView settingsView;
    SlotRackComponent slotRack;

    void updateActiveTabView(ActiveTab tab);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DrmtrgrrAudioProcessorEditor)
};

} // namespace yeahwaah
