#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "HardwareControls.h"

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
    explicit HeaderBar(DrmtrgrrAudioProcessor& processor);
    ~HeaderBar() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void updateMeters(float inPeak, float outPeak);

    std::function<void(ActiveTab)> onTabChanged;

    void setActiveTab(ActiveTab tab);
    ActiveTab getActiveTab() const { return currentTab; }

private:
    DrmtrgrrAudioProcessor& processor;

    // Browser / Settings views toggle over the main panels; Trigger is the default view.
    juce::TextButton browserTabButton { "BROWSER" };
    juce::TextButton settingsTabButton { "SETTINGS" };
    ActiveTab currentTab = ActiveTab::Trigger;

    juce::ComboBox presetBox;
    juce::ArrowButton prevPresetBtn { "prev", 0.5f, juce::Colour(0xffe9dfc6) };
    juce::ArrowButton nextPresetBtn { "next", 0.0f, juce::Colour(0xffe9dfc6) };

    hw::HardwareKnob inputKnob;
    hw::HardwareKnob mixKnob;
    hw::HardwareKnob outputKnob;

    float currentInPeak = 0.0f;
    float currentOutPeak = 0.0f;

    void updateTabButtons();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(HeaderBar)
};

} // namespace yeahwaah
