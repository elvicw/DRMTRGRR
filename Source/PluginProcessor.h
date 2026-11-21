#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include "DSP/TransientDetector.h"
#include "DSP/SampleSlot.h"
#include "DSP/TransientGate.h"
#include "DSP/VisualizerBuffer.h"

namespace yeahwaah
{

class DrmtrgrrAudioProcessor : public juce::AudioProcessor
{
public:
    static constexpr int NumSlots = 6;

    DrmtrgrrAudioProcessor();
    ~DrmtrgrrAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState& getAPVTS() { return apvts; }
    VisualizerBuffer& getVisualizerBuffer() { return visualizer; }
    SampleSlot& getSlot(int index) { return slots[juce::jlimit(0, NumSlots - 1, index)]; }
    juce::AudioFormatManager& getFormatManager() { return formatManager; }

    float getOutputPeak() const { return masterOutputPeak.load(std::memory_order_relaxed); }
    float getInputPeak() const { return detector.getCurrentPeak(); }

    void triggerSlotManually(int slotIndex, float velocity = 1.0f);
    void applyFactoryPreset(int presetIndex);

private:
    juce::AudioProcessorValueTreeState apvts;
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    TransientDetector detector;
    TransientGate gate;
    SampleSlot slots[NumSlots];
    VisualizerBuffer visualizer;
    juce::AudioFormatManager formatManager;

    juce::AudioBuffer<float> dryCopyBuffer;
    juce::AudioBuffer<float> auditionBuffer;
    juce::AudioBuffer<float> wetSampleBuffer;

    std::vector<TriggerEvent> detectedTriggers;
    std::atomic<float> masterOutputPeak { 0.0f };

    int currentProgramIndex = 0;
    static const juce::StringArray presetNames;

    // MIDI Note Off scheduling
    struct PendingMidiNoteOff
    {
        int noteNumber;
        int channel;
        int samplesRemaining;
    };
    std::vector<PendingMidiNoteOff> pendingNoteOffs;

    void updateDSPParameters();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DrmtrgrrAudioProcessor)
};

} // namespace yeahwaah
