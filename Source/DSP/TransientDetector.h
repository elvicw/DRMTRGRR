#pragma once
#include <juce_core/juce_core.h>
#include <juce_dsp/juce_dsp.h>
#include "VisualizerBuffer.h"

namespace yeahwaah
{

struct TriggerEvent
{
    int sampleOffset;
    float velocity; // 0.0 to 1.0
};

class TransientDetector
{
public:
    TransientDetector();
    ~TransientDetector() = default;

    void prepare(double sampleRate, int samplesPerBlock);
    void reset();

    // Sets detection parameters
    void setInputGain(float gainDb);
    void setDetail(float detailDb);          // Threshold: -60 dB to 0 dB
    void setSensitivity(float sensitivityPct); // 0 to 100%
    void setRetriggerMs(float retriggerMs);   // 5 to 500 ms
    void setDynamics(float dynamicsPct);      // 0 to 100%
    void setFilterFrequencies(float lowCutHz, float highCutHz);
    void setAuditionFilter(bool audition);
    void setLeakageSuppression(bool enable, float amountPct);

    // Processes an audio buffer. Returns detected trigger events.
    // If audition mode is ON, filteredBuffer receives the audition audio.
    void process(const juce::AudioBuffer<float>& inputBuffer,
                 juce::AudioBuffer<float>& auditionBuffer,
                 std::vector<TriggerEvent>& detectedTriggers,
                 VisualizerBuffer& visualizer);

    float getCurrentPeak() const { return currentPeak.load(std::memory_order_relaxed); }
    bool isAuditioning() const { return auditionFilter; }

private:
    double currentSampleRate = 44100.0;
    float inputGainLinear = 1.0f;
    float detailDb = -24.0f;
    float detailLinear = 0.063f;
    float sensitivity = 0.8f;
    float retriggerMs = 40.0f;
    float dynamics = 0.7f;
    float lowCutFreq = 60.0f;
    float highCutFreq = 4000.0f;
    bool auditionFilter = false;
    bool leakageEnabled = false;
    float leakageAmount = 0.5f;

    // Filter chain using JUCE DSP IIR filters
    using Filter = juce::dsp::IIR::Filter<float>;
    Filter lowCutFilterLeft, lowCutFilterRight;
    Filter highCutFilterLeft, highCutFilterRight;

    // Transient envelope tracking
    float envFast = 0.0f;
    float envSlow = 0.0f;
    float alphaFast = 0.0f;
    float alphaSlow = 0.0f;

    // Leakage / background estimator
    float envLeakage = 0.0f;
    float alphaLeakage = 0.0f;

    // Lockout & peak tracking
    int lockoutSamplesRemaining = 0;
    int retriggerSamples = 1764; // default at 44.1k
    int peakHoldSamplesRemaining = 0;
    float peakValueInWindow = 0.0f;
    int pendingTriggerSampleOffset = -1;

    std::atomic<float> currentPeak { 0.0f };

    void updateFilterCoefficients();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TransientDetector)
};

} // namespace yeahwaah
