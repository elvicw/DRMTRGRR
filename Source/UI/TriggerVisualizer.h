#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "../DSP/VisualizerBuffer.h"

namespace yeahwaah
{

struct HitShockwave
{
    float x;
    float y;
    float velocity;
    int64_t timestamp;
};

class TriggerVisualizer : public juce::Component,
                          public juce::FileDragAndDropTarget
{
public:
    TriggerVisualizer(VisualizerBuffer& visualizerBuffer,
                      juce::RangedAudioParameter* detailParameter,
                      juce::RangedAudioParameter* lowCutParameter = nullptr,
                      juce::RangedAudioParameter* highCutParameter = nullptr,
                      juce::RangedAudioParameter* retriggerParameter = nullptr);
    ~TriggerVisualizer() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;

    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;
    void mouseUp(const juce::MouseEvent& e) override;
    void mouseMove(const juce::MouseEvent& e) override;

    // File Drag and Drop directly onto the Visualizer
    bool isInterestedInFileDrag(const juce::StringArray& files) override;
    void fileDragEnter(const juce::StringArray& files, int x, int y) override;
    void fileDragExit(const juce::StringArray& files) override;
    void filesDropped(const juce::StringArray& files, int x, int y) override;

    void updateWaveform();
    void setAuditionMode(bool auditioning) { isAuditioning = auditioning; }
    int getActiveHitCount() const;

    std::function<void(const juce::File&)> onSampleDropped;

private:
    VisualizerBuffer& visualizer;
    juce::RangedAudioParameter* detailParam = nullptr;
    juce::RangedAudioParameter* lowCutParam = nullptr;
    juce::RangedAudioParameter* highCutParam = nullptr;
    juce::RangedAudioParameter* retriggerParam = nullptr;

    std::vector<float> waveformData;
    std::vector<HitEvent> hitMarkers;
    std::vector<HitShockwave> shockwaves;

    enum class DragTarget
    {
        None,
        Detail,
        LowCut,
        HighCut
    };
    DragTarget activeDragTarget = DragTarget::None;

    bool isAuditioning = false;
    bool isDragHovered = false;

    float detailDb = -24.0f;
    float lowCutHz = 60.0f;
    float highCutHz = 4500.0f;
    float retriggerMs = 35.0f;

    int64_t lastSeenHitTimestamp = 0;

    float dbToY(float dB, float height) const;
    float yToDb(float y, float height) const;

    float hzToX(float hz, float startX, float endX) const;
    float xToHz(float x, float startX, float endX) const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TriggerVisualizer)
};

} // namespace yeahwaah
