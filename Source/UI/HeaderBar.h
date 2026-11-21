#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>

namespace yeahwaah
{

class DrmtrgrrAudioProcessor;

class HeaderBar : public juce::Component
{
public:
    HeaderBar(DrmtrgrrAudioProcessor& processor);
    ~HeaderBar() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void updateMeters(float inPeak, float outPeak);

private:
    DrmtrgrrAudioProcessor& processor;

    juce::ComboBox presetBox;
    juce::Slider mixSlider;
    juce::Slider outputGainSlider;
    juce::Label mixLabel, outLabel;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> mixAtt;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> outGainAtt;

    float currentInPeak = 0.0f;
    float currentOutPeak = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(HeaderBar)
};

} // namespace yeahwaah
