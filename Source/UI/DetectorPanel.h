#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>

namespace yeahwaah
{

class DetectorPanel : public juce::Component
{
public:
    DetectorPanel(juce::AudioProcessorValueTreeState& apvts);
    ~DetectorPanel() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    juce::AudioProcessorValueTreeState& apvts;

    // Transient Detection Knobs
    juce::Slider inputGainSlider;
    juce::Slider detailSlider;
    juce::Slider sensitivitySlider;
    juce::Slider retriggerSlider;
    juce::Slider dynamicsSlider;

    // Filter Controls
    juce::Slider lowCutSlider;
    juce::Slider highCutSlider;
    juce::TextButton auditionButton { "LISTEN" };

    // Gate Controls
    juce::ToggleButton gateEnableButton { "GATE" };
    juce::Slider gateThresholdSlider;
    juce::Slider gateAttackSlider;
    juce::Slider gateHoldSlider;
    juce::Slider gateReleaseSlider;

    // Leakage & MIDI
    juce::ToggleButton leakageEnableButton { "BLEED CANCEL" };
    juce::Slider leakageAmountSlider;
    juce::ToggleButton midiOutButton { "MIDI OUT" };
    juce::Slider midiNoteSlider;
    juce::Slider midiChannelSlider;

    // Labels
    juce::Label inputGainLabel, detailLabel, sensLabel, retriggerLabel, dynLabel;
    juce::Label lowCutLabel, highCutLabel;
    juce::Label gateThreshLabel, gateAttLabel, gateHldLabel, gateRelLabel;
    juce::Label leakLabel, midiNoteLabel, midiChLabel;

    // Attachments
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;

    std::unique_ptr<SliderAttachment> inputGainAtt;
    std::unique_ptr<SliderAttachment> detailAtt;
    std::unique_ptr<SliderAttachment> sensAtt;
    std::unique_ptr<SliderAttachment> retriggerAtt;
    std::unique_ptr<SliderAttachment> dynAtt;

    std::unique_ptr<SliderAttachment> lowCutAtt;
    std::unique_ptr<SliderAttachment> highCutAtt;
    std::unique_ptr<ButtonAttachment> auditionAtt;

    std::unique_ptr<ButtonAttachment> gateEnableAtt;
    std::unique_ptr<SliderAttachment> gateThreshAtt;
    std::unique_ptr<SliderAttachment> gateAttAtt;
    std::unique_ptr<SliderAttachment> gateHldAtt;
    std::unique_ptr<SliderAttachment> gateRelAtt;

    std::unique_ptr<ButtonAttachment> leakageEnableAtt;
    std::unique_ptr<SliderAttachment> leakageAmountAtt;
    std::unique_ptr<ButtonAttachment> midiOutAtt;
    std::unique_ptr<SliderAttachment> midiNoteAtt;
    std::unique_ptr<SliderAttachment> midiChannelAtt;

    void setupKnob(juce::Slider& s, juce::Label& l, const juce::String& text, const juce::String& suffix);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DetectorPanel)
};

} // namespace yeahwaah
