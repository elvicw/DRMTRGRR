#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "../DSP/VisualizerBuffer.h"

namespace yeahwaah
{

class TriggerVisualizer : public juce::Component
{
public:
    TriggerVisualizer(VisualizerBuffer& visualizerBuffer, juce::RangedAudioParameter* detailParameter);
    ~TriggerVisualizer() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;

    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;
    void mouseUp(const juce::MouseEvent& e) override;

    void updateWaveform();
    void setAuditionMode(bool auditioning) { isAuditioning = auditioning; }

private:
    VisualizerBuffer& visualizer;
    juce::RangedAudioParameter* detailParam;

    std::vector<float> waveformData;
    std::vector<HitEvent> hitMarkers;

    bool isDraggingDetail = false;
    bool isAuditioning = false;

    float detailDb = -24.0f;

    float dbToY(float dB, float height) const;
    float yToDb(float y, float height) const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TriggerVisualizer)
};

} // namespace yeahwaah
