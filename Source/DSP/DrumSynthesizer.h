#pragma once
#include <juce_audio_basics/juce_audio_basics.h>

namespace yeahwaah
{

enum class FactoryDrumType
{
    PunchKick = 0,
    SnareCrack,
    RimBite,
    SubThud,
    RoomAmbience,
    SnareCrisp,
    Count
};

class DrumSynthesizer
{
public:
    static juce::AudioBuffer<float> generateSample(FactoryDrumType type, double sampleRate);
    static juce::String getSampleName(FactoryDrumType type);

private:
    static void generateKick(juce::AudioBuffer<float>& buffer, double sampleRate);
    static void generateSnare(juce::AudioBuffer<float>& buffer, double sampleRate);
    static void generateRim(juce::AudioBuffer<float>& buffer, double sampleRate);
    static void generateSub(juce::AudioBuffer<float>& buffer, double sampleRate);
    static void generateRoom(juce::AudioBuffer<float>& buffer, double sampleRate);
    static void generateCrisp(juce::AudioBuffer<float>& buffer, double sampleRate);
};

} // namespace yeahwaah
