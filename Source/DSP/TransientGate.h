#pragma once
#include <juce_core/juce_core.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <cmath>

namespace yeahwaah
{

class TransientGate
{
public:
    TransientGate() = default;

    void prepare(double sampleRate)
    {
        currentSampleRate = sampleRate > 0.0 ? sampleRate : 44100.0;
        reset();
    }

    void reset()
    {
        gateGain = 0.0f;
        state = State::Closed;
        holdSamplesRemaining = 0;
        updateTimeConstants();
    }

    void setParameters(bool enabled, float thresholdDb, float attackMs, float holdMs, float releaseMs)
    {
        isEnabled = enabled;
        thresholdLinear = juce::Decibels::decibelsToGain(thresholdDb);
        attMs = juce::jlimit(0.1f, 100.0f, attackMs);
        hldMs = juce::jlimit(1.0f, 1000.0f, holdMs);
        relMs = juce::jlimit(5.0f, 2000.0f, releaseMs);
        updateTimeConstants();
    }

    void process(juce::AudioBuffer<float>& buffer)
    {
        if (!isEnabled)
            return;

        const int numSamples = buffer.getNumSamples();
        const int numChannels = buffer.getNumChannels();
        if (numSamples == 0) return;

        float* l = buffer.getWritePointer(0);
        float* r = numChannels > 1 ? buffer.getWritePointer(1) : nullptr;

        for (int i = 0; i < numSamples; ++i)
        {
            float inputLevel = std::abs(l[i]);
            if (r != nullptr)
                inputLevel = std::max(inputLevel, std::abs(r[i]));

            switch (state)
            {
                case State::Closed:
                    if (inputLevel >= thresholdLinear)
                    {
                        state = State::Attack;
                    }
                    break;

                case State::Attack:
                    gateGain += attackStep;
                    if (gateGain >= 1.0f)
                    {
                        gateGain = 1.0f;
                        state = State::Hold;
                        holdSamplesRemaining = totalHoldSamples;
                    }
                    break;

                case State::Hold:
                    if (inputLevel >= thresholdLinear)
                        holdSamplesRemaining = totalHoldSamples; // retrigger hold
                    else if (--holdSamplesRemaining <= 0)
                        state = State::Release;
                    break;

                case State::Release:
                    if (inputLevel >= thresholdLinear)
                    {
                        state = State::Attack;
                    }
                    else
                    {
                        gateGain *= releaseCoeff;
                        if (gateGain <= 0.001f)
                        {
                            gateGain = 0.0f;
                            state = State::Closed;
                        }
                    }
                    break;
            }

            l[i] *= gateGain;
            if (r != nullptr)
                r[i] *= gateGain;
        }
    }

private:
    enum class State { Closed, Attack, Hold, Release };

    bool isEnabled = false;
    double currentSampleRate = 44100.0;
    float thresholdLinear = 0.05f;
    float attMs = 1.0f;
    float hldMs = 50.0f;
    float relMs = 100.0f;

    State state = State::Closed;
    float gateGain = 0.0f;
    float attackStep = 0.05f;
    int holdSamplesRemaining = 0;
    int totalHoldSamples = 2205;
    float releaseCoeff = 0.999f;

    void updateTimeConstants()
    {
        int attackSamples = std::max(1, static_cast<int>((attMs / 1000.0f) * currentSampleRate));
        attackStep = 1.0f / static_cast<float>(attackSamples);

        totalHoldSamples = static_cast<int>((hldMs / 1000.0f) * currentSampleRate);

        releaseCoeff = static_cast<float>(std::exp(-1.0 / (currentSampleRate * (relMs / 1000.0f))));
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TransientGate)
};

} // namespace yeahwaah
