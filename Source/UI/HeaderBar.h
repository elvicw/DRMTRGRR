#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>

namespace yeahwaah
{

class DrmtrgrrAudioProcessor;

enum class ActiveTab
{
    Browser = 0,
    Trigger,
    Settings
};

class HeaderBar : public juce::Component
{
public:
    HeaderBar(DrmtrgrrAudioProcessor& processor);
    ~HeaderBar() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void updateMeters(float inPeak, float outPeak);

    std::function<void(ActiveTab)> onTabChanged;

    void setActiveTab(ActiveTab tab);
    ActiveTab getActiveTab() const { return currentTab; }

private:
    DrmtrgrrAudioProcessor& processor;

    // Slate Trigger 2 Navigation Tabs
    juce::TextButton browserTabButton { "BROWSER" };
    juce::TextButton triggerTabButton { "TRIGGER" };
    juce::TextButton settingsTabButton { "SETTINGS" };
    ActiveTab currentTab = ActiveTab::Trigger;

    // Presets
    juce::TextButton prevPresetBtn { "◀" };
    juce::ComboBox presetBox;
    juce::TextButton nextPresetBtn { "▶" };

    // Master Knobs
    juce::Slider inputGainSlider;
    juce::Slider mixSlider;
    juce::Slider outputGainSlider;
    juce::Label inLabel, mixLabel, outLabel;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> inGainAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> mixAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> outGainAtt;

    float currentInPeak = 0.0f;
    float currentOutPeak = 0.0f;

    void updateTabButtons();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(HeaderBar)
};

} // namespace yeahwaah
