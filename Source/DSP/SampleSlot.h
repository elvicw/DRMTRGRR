#pragma once
#include <juce_core/juce_core.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include "DrumSynthesizer.h"
#include <atomic>
#include <vector>

namespace yeahwaah
{

class SampleSlot
{
public:
    SampleSlot(int slotIndex = 0);
    ~SampleSlot() = default;

    void prepare(double sampleRate, int samplesPerBlock);
    void reset();

    // Configuration & parameters
    void setEnabled(bool enabled) { isEnabled = enabled; }
    bool getEnabled() const { return isEnabled; }

    void setVolume(float volumeDb);
    void setPan(float pan);              // -1.0 (Left) to +1.0 (Right)
    void setTune(float semitones);       // -24.0 to +24.0
    void setPhaseInvert(bool invert)     { phaseInvert = invert; }
    void setSolo(bool solo)              { isSolo = solo; }
    bool getSolo() const                 { return isSolo; }
    void setSlotIndex(int index)         { slotIndex = index; }
    int getSlotIndex() const             { return slotIndex; }
    void setMute(bool mute)              { isMute = mute; }
    bool getMute() const                 { return isMute; }

    void setVelocityRange(float minVel, float maxVel); // 0.0 to 1.0
    void setDynamics(float dynamicsPct);
    void setEnvelope(float attackMs, float decayMs);

    // Audio sample loading
    bool loadFromFile(const juce::File& file, juce::AudioFormatManager& formatManager);
    void loadFactoryDrum(FactoryDrumType type);
    juce::String getSampleName() const { return sampleName; }
    bool hasSample() const { return sampleBuffer.getNumSamples() > 0; }

    // Playback
    void trigger(float velocity, int sampleOffset = 0);
    void stop();

    // Renders audio into destination mix buffer (additive)
    void render(juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples, bool anySoloActive);

    float getCurrentPeakMeter() const { return peakMeter.load(std::memory_order_relaxed); }
    void getWaveformThumbnail(std::vector<float>& points, int numPoints = 64) const;

private:
    int slotIndex = 0;
    double currentSampleRate = 44100.0;

    bool isEnabled = true;
    bool isSolo = false;
    bool isMute = false;
    bool phaseInvert = false;

    float volumeDb = 0.0f;
    float gainLinear = 1.0f;
    float pan = 0.0f; // -1 to +1
    float panL = 0.707f;
    float panR = 0.707f;

    float tuneSemitones = 0.0f;
    double pitchRatio = 1.0;

    float minVelocity = 0.0f;
    float maxVelocity = 1.0f;
    float dynamics = 0.8f;

    float attackTimeMs = 0.5f;
    float decayTimeMs = 350.0f;

    juce::String sampleName;
    juce::AudioBuffer<float> sampleBuffer;
    juce::CriticalSection bufferLock;

    // Active playback voice state
    struct Voice
    {
        bool active = false;
        double currentPosition = 0.0;
        float currentVelocity = 1.0f;
        float envLevel = 0.0f;
        enum class EnvStage { Attack, Decay, Idle } stage = EnvStage::Idle;
        float attackIncrement = 0.01f;
        float decayMultiplier = 0.999f;
    };

    static constexpr int maxPolyphony = 4;
    Voice voices[maxPolyphony];

    std::atomic<float> peakMeter { 0.0f };

    void updatePanGains();
    void updatePitchRatio();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SampleSlot)
};

} // namespace yeahwaah
