#include "DrumSynthesizer.h"
#include <cmath>
#include <random>

namespace yeahwaah
{

juce::String DrumSynthesizer::getSampleName(FactoryDrumType type)
{
    switch (type)
    {
        case FactoryDrumType::PunchKick:    return "Direct Punch Kick";
        case FactoryDrumType::SnareCrack:   return "Snare Crack Top";
        case FactoryDrumType::RimBite:      return "Rim / Attack Bite";
        case FactoryDrumType::SubThud:      return "Deep Sub Thud";
        case FactoryDrumType::RoomAmbience: return "Room Ambience Tail";
        case FactoryDrumType::SnareCrisp:   return "Snare Wires Crisp";
        default:                            return "Drum Sample";
    }
}

juce::AudioBuffer<float> DrumSynthesizer::generateSample(FactoryDrumType type, double sampleRate)
{
    juce::AudioBuffer<float> buffer;
    switch (type)
    {
        case FactoryDrumType::PunchKick:    generateKick(buffer, sampleRate); break;
        case FactoryDrumType::SnareCrack:   generateSnare(buffer, sampleRate); break;
        case FactoryDrumType::RimBite:      generateRim(buffer, sampleRate); break;
        case FactoryDrumType::SubThud:      generateSub(buffer, sampleRate); break;
        case FactoryDrumType::RoomAmbience: generateRoom(buffer, sampleRate); break;
        case FactoryDrumType::SnareCrisp:   generateCrisp(buffer, sampleRate); break;
        default:                            generateKick(buffer, sampleRate); break;
    }
    return buffer;
}

void DrumSynthesizer::generateKick(juce::AudioBuffer<float>& buffer, double sampleRate)
{
    const int numSamples = static_cast<int>(sampleRate * 0.45); // 450 ms
    buffer.setSize(2, numSamples);
    buffer.clear();

    float* l = buffer.getWritePointer(0);
    float* r = buffer.getWritePointer(1);

    double phase = 0.0;
    std::mt19937 rng(42);
    std::uniform_real_distribution<float> dist(-1.0f, 1.0f);

    for (int i = 0; i < numSamples; ++i)
    {
        double t = static_cast<double>(i) / sampleRate;

        // Exponential pitch drop from 180Hz down to 48Hz
        double pitch = 48.0 + 132.0 * std::exp(-t * 28.0) + 25.0 * std::exp(-t * 6.0);
        double phaseDelta = (pitch * 2.0 * juce::MathConstants<double>::pi) / sampleRate;
        phase += phaseDelta;

        // Amplitude envelope: instant attack, decay
        double env = std::exp(-t * 8.5);

        // Click transient for beater attack
        float click = 0.0f;
        if (i < static_cast<int>(sampleRate * 0.005)) // 5ms click
        {
            float clickEnv = 1.0f - static_cast<float>(i) / (sampleRate * 0.005f);
            click = dist(rng) * clickEnv * 0.45f;
        }

        // Body sine with gentle soft saturation
        float body = static_cast<float>(std::sin(phase) * env);
        float sample = std::tanh((body + click) * 1.35f);

        l[i] = sample;
        r[i] = sample;
    }
}

void DrumSynthesizer::generateSnare(juce::AudioBuffer<float>& buffer, double sampleRate)
{
    const int numSamples = static_cast<int>(sampleRate * 0.40); // 400 ms
    buffer.setSize(2, numSamples);
    buffer.clear();

    float* l = buffer.getWritePointer(0);
    float* r = buffer.getWritePointer(1);

    double tonePhase = 0.0;
    std::mt19937 rng(101);
    std::uniform_real_distribution<float> dist(-1.0f, 1.0f);

    float noiseHpPrevL = 0.0f;
    float noiseHpPrevR = 0.0f;

    for (int i = 0; i < numSamples; ++i)
    {
        double t = static_cast<double>(i) / sampleRate;

        // Snare body fundamental (220 Hz dropping to 160 Hz)
        double pitch = 160.0 + 60.0 * std::exp(-t * 30.0);
        tonePhase += (pitch * 2.0 * juce::MathConstants<double>::pi) / sampleRate;
        double toneEnv = std::exp(-t * 18.0);
        float tone = static_cast<float>(std::sin(tonePhase) * toneEnv * 0.7);

        // Snare wires (high-pass filtered noise)
        double wireEnv = std::exp(-t * 11.0);
        float rawNoiseL = dist(rng);
        float rawNoiseR = dist(rng);

        // Simple high-pass filter on noise
        float hpNoiseL = rawNoiseL - 0.75f * noiseHpPrevL;
        noiseHpPrevL = rawNoiseL;
        float hpNoiseR = rawNoiseR - 0.75f * noiseHpPrevR;
        noiseHpPrevR = rawNoiseR;

        float wireL = hpNoiseL * static_cast<float>(wireEnv) * 0.65f;
        float wireR = hpNoiseR * static_cast<float>(wireEnv) * 0.65f;

        // Initial crack pop
        float crack = 0.0f;
        if (i < static_cast<int>(sampleRate * 0.003))
        {
            float crackEnv = 1.0f - static_cast<float>(i) / (sampleRate * 0.003f);
            crack = dist(rng) * crackEnv * 0.6f;
        }

        l[i] = std::tanh((tone + wireL + crack) * 1.2f);
        r[i] = std::tanh((tone + wireR + crack) * 1.2f);
    }
}

void DrumSynthesizer::generateRim(juce::AudioBuffer<float>& buffer, double sampleRate)
{
    const int numSamples = static_cast<int>(sampleRate * 0.25); // 250 ms
    buffer.setSize(2, numSamples);
    buffer.clear();

    float* l = buffer.getWritePointer(0);
    float* r = buffer.getWritePointer(1);

    double p1 = 0.0, p2 = 0.0, p3 = 0.0;
    std::mt19937 rng(303);
    std::uniform_real_distribution<float> dist(-1.0f, 1.0f);

    for (int i = 0; i < numSamples; ++i)
    {
        double t = static_cast<double>(i) / sampleRate;

        // Woodblock / rim metallic harmonics (850Hz, 1680Hz, 2400Hz)
        p1 += (850.0 * 2.0 * juce::MathConstants<double>::pi) / sampleRate;
        p2 += (1680.0 * 2.0 * juce::MathConstants<double>::pi) / sampleRate;
        p3 += (2400.0 * 2.0 * juce::MathConstants<double>::pi) / sampleRate;

        double env = std::exp(-t * 32.0);
        float harmonics = static_cast<float>((std::sin(p1) * 0.5 + std::sin(p2) * 0.35 + std::sin(p3) * 0.15) * env);

        float click = 0.0f;
        if (i < static_cast<int>(sampleRate * 0.002))
        {
            float cEnv = 1.0f - static_cast<float>(i) / (sampleRate * 0.002f);
            click = dist(rng) * cEnv * 0.8f;
        }

        float sample = std::tanh((harmonics + click) * 1.4f);
        l[i] = sample;
        r[i] = sample;
    }
}

void DrumSynthesizer::generateSub(juce::AudioBuffer<float>& buffer, double sampleRate)
{
    const int numSamples = static_cast<int>(sampleRate * 0.60); // 600 ms
    buffer.setSize(2, numSamples);
    buffer.clear();

    float* l = buffer.getWritePointer(0);
    float* r = buffer.getWritePointer(1);

    double phase = 0.0;
    for (int i = 0; i < numSamples; ++i)
    {
        double t = static_cast<double>(i) / sampleRate;
        double pitch = 44.0 + 20.0 * std::exp(-t * 12.0); // 64Hz down to 44Hz
        phase += (pitch * 2.0 * juce::MathConstants<double>::pi) / sampleRate;
        double env = std::exp(-t * 5.0);

        float sample = static_cast<float>(std::sin(phase) * env * 0.95);
        l[i] = sample;
        r[i] = sample;
    }
}

void DrumSynthesizer::generateRoom(juce::AudioBuffer<float>& buffer, double sampleRate)
{
    const int numSamples = static_cast<int>(sampleRate * 0.65); // 650 ms
    buffer.setSize(2, numSamples);
    buffer.clear();

    float* l = buffer.getWritePointer(0);
    float* r = buffer.getWritePointer(1);

    std::mt19937 rngL(555), rngR(777);
    std::uniform_real_distribution<float> dist(-1.0f, 1.0f);

    float filterL = 0.0f;
    float filterR = 0.0f;

    for (int i = 0; i < numSamples; ++i)
    {
        double t = static_cast<double>(i) / sampleRate;
        // Room burst envelope with pre-delay
        double attack = std::min(1.0, t / 0.015);
        double decay = std::exp(-t * 6.2);
        float roomEnv = static_cast<float>(attack * decay);

        // Low-pass filtered noise to simulate warm room reflections
        float nL = dist(rngL);
        float nR = dist(rngR);
        filterL += 0.22f * (nL - filterL);
        filterR += 0.22f * (nR - filterR);

        l[i] = filterL * roomEnv * 0.75f;
        r[i] = filterR * roomEnv * 0.75f;
    }
}

void DrumSynthesizer::generateCrisp(juce::AudioBuffer<float>& buffer, double sampleRate)
{
    const int numSamples = static_cast<int>(sampleRate * 0.35); // 350 ms
    buffer.setSize(2, numSamples);
    buffer.clear();

    float* l = buffer.getWritePointer(0);
    float* r = buffer.getWritePointer(1);

    std::mt19937 rng(888);
    std::uniform_real_distribution<float> dist(-1.0f, 1.0f);

    float bandpassL = 0.0f, bandpassR = 0.0f;
    for (int i = 0; i < numSamples; ++i)
    {
        double t = static_cast<double>(i) / sampleRate;
        double env = std::exp(-t * 14.0);

        float nL = dist(rng);
        float nR = dist(rng);

        // High frequency bandpass buzz
        bandpassL = 0.8f * bandpassL + 0.2f * nL;
        bandpassR = 0.8f * bandpassR + 0.2f * nR;

        float crispL = (nL - bandpassL) * static_cast<float>(env) * 0.7f;
        float crispR = (nR - bandpassR) * static_cast<float>(env) * 0.7f;

        l[i] = crispL;
        r[i] = crispR;
    }
}

} // namespace yeahwaah
