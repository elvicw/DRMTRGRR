#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "../PluginProcessor.h"

namespace yeahwaah
{

class SettingsView : public juce::Component
{
public:
    SettingsView(DrmtrgrrAudioProcessor& processor);
    ~SettingsView() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    DrmtrgrrAudioProcessor& processor;

    juce::Label headerLabel;
    juce::Label versionLabel;
    juce::Label infoText;

    juce::Label velCurveLabel;
    juce::ComboBox velCurveBox;

    juce::TextButton restoreDefaultsButton { "RESTORE FACTORY DEFAULTS" };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SettingsView)
};

} // namespace yeahwaah
