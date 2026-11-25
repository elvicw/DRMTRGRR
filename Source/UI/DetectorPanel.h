#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "HardwareControls.h"

namespace yeahwaah
{

// Trigger detection, oscilloscope frame, meters and the four processing modules.
// Laid out in design-space units; the panel's own origin sits at kOriginY of the editor content.
class DetectorPanel : public juce::Component
{
public:
    static constexpr int kOriginY = 128;
    static juce::Rectangle<int> getScopeBounds() { return { 420, 172, 902, 200 }; }

    explicit DetectorPanel(juce::AudioProcessorValueTreeState& apvts);
    ~DetectorPanel() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;

    void setMeters(float inputPeak, float outputPeak);
    void setActiveHits(int count);

private:
    juce::AudioProcessorValueTreeState& apvts;

    hw::HardwareKnob detailKnob, sensitivityKnob;
    hw::ValuePad retrigPad, dynamicsPad;

    hw::HardwareKnob trimKnob, engineDetailKnob, engineDynamicsKnob;
    hw::HardwareKnob lowCutKnob, highCutKnob;
    hw::HardwareButton listenButton { "Listen", hw::HardwareButton::Style::Lamp };

    hw::HardwareButton gateEnableButton { "Gate", hw::HardwareButton::Style::Led };
    hw::HardwareKnob gateThresholdKnob, gateAttackKnob, gateHoldKnob, gateReleaseKnob;

    hw::HardwareButton bleedEnableButton { "Bleed", hw::HardwareButton::Style::Led };
    hw::HardwareKnob bleedKnob;
    hw::HardwareButton midiOutButton { "MidiOut", hw::HardwareButton::Style::Lamp };
    juce::TextButton midiSettingsButton { "MIDI OUT" };

    juce::Slider thresholdBar;

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;
    std::unique_ptr<SliderAttachment> thresholdBarAtt;
    std::unique_ptr<ButtonAttachment> listenAtt, gateEnableAtt, bleedEnableAtt, midiOutAtt;

    float inMeter = 0.0f, outMeter = 0.0f;
    int activeHits = 0;

    void showMidiSettings();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DetectorPanel)
};

} // namespace yeahwaah
