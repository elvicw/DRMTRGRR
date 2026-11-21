#include "SampleSlot.h"
#include <cmath>

namespace yeahwaah
{

SampleSlot::SampleSlot(int index)
    : slotIndex(index)
{
    updatePanGains();
    updatePitchRatio();
}

void SampleSlot::prepare(double sampleRate, int /*samplesPerBlock*/)
{
    currentSampleRate = sampleRate > 0.0 ? sampleRate : 44100.0;
    reset();

    // If no sample is loaded yet, load default factory drum for this slot
    if (sampleBuffer.getNumSamples() == 0)
    {
        auto drumType = static_cast<FactoryDrumType>(slotIndex % static_cast<int>(FactoryDrumType::Count));
        loadFactoryDrum(drumType);
    }
}

void SampleSlot::reset()
{
    for (auto& v : voices)
    {
        v.active = false;
        v.stage = Voice::EnvStage::Idle;
        v.envLevel = 0.0f;
    }
    peakMeter.store(0.0f, std::memory_order_relaxed);
}

void SampleSlot::setVolume(float dB)
{
    volumeDb = juce::jlimit(-60.0f, 12.0f, dB);
    gainLinear = (volumeDb <= -59.5f) ? 0.0f : juce::Decibels::decibelsToGain(volumeDb);
}

void SampleSlot::setPan(float newPan)
{
    pan = juce::jlimit(-1.0f, 1.0f, newPan);
    updatePanGains();
}

void SampleSlot::updatePanGains()
{
    // Constant power panning
    float angle = (pan + 1.0f) * 0.25f * juce::MathConstants<float>::pi; // 0 to pi/2
    panL = std::cos(angle);
    panR = std::sin(angle);
}

void SampleSlot::setTune(float semitones)
{
    tuneSemitones = juce::jlimit(-24.0f, 24.0f, semitones);
    updatePitchRatio();
}

void SampleSlot::updatePitchRatio()
{
    pitchRatio = std::pow(2.0, tuneSemitones / 12.0);
}

void SampleSlot::setVelocityRange(float minVel, float maxVel)
{
    minVelocity = juce::jlimit(0.0f, 1.0f, minVel);
    maxVelocity = juce::jlimit(minVelocity, 1.0f, maxVel);
}

void SampleSlot::setDynamics(float dynamicsPct)
{
    dynamics = juce::jlimit(0.0f, 100.0f, dynamicsPct) / 100.0f;
}

void SampleSlot::setEnvelope(float attackMs, float decayMs)
{
    attackTimeMs = juce::jlimit(0.1f, 100.0f, attackMs);
    decayTimeMs = juce::jlimit(10.0f, 3000.0f, decayMs);
}

bool SampleSlot::loadFromFile(const juce::File& file, juce::AudioFormatManager& formatManager)
{
    std::unique_ptr<juce::AudioFormatReader> reader(formatManager.createReaderFor(file));
    if (reader == nullptr)
        return false;

    const juce::ScopedLock sl(bufferLock);
    sampleBuffer.setSize((int)reader->numChannels, (int)reader->lengthInSamples);
    reader->read(&sampleBuffer, 0, (int)reader->lengthInSamples, 0, true, true);
    sampleName = file.getFileNameWithoutExtension();
    reset();
    return true;
}

void SampleSlot::loadFactoryDrum(FactoryDrumType type)
{
    const juce::ScopedLock sl(bufferLock);
    sampleBuffer = DrumSynthesizer::generateSample(type, currentSampleRate);
    sampleName = DrumSynthesizer::getSampleName(type);
    reset();
}

void SampleSlot::trigger(float velocity, int /*sampleOffset*/)
{
    if (!isEnabled || isMute)
        return;

    // Velocity window gating
    if (velocity < minVelocity || velocity > maxVelocity)
        return;

    const juce::ScopedLock sl(bufferLock);
    if (sampleBuffer.getNumSamples() == 0)
        return;

    // Dynamics scaling: 0 means constant velocity 1.0; 1 means full dynamic response
    float effectiveVel = (1.0f - dynamics) + dynamics * velocity;
    effectiveVel = juce::jlimit(0.05f, 1.0f, effectiveVel);

    // Find available voice or steal oldest
    int targetVoiceIdx = 0;
    double maxPos = -1.0;
    for (int i = 0; i < maxPolyphony; ++i)
    {
        if (!voices[i].active)
        {
            targetVoiceIdx = i;
            break;
        }
        if (voices[i].currentPosition > maxPos)
        {
            maxPos = voices[i].currentPosition;
            targetVoiceIdx = i;
        }
    }

    auto& v = voices[targetVoiceIdx];
    v.active = true;
    v.currentPosition = 0.0;
    v.currentVelocity = effectiveVel;
    v.stage = Voice::EnvStage::Attack;
    v.envLevel = 0.0f;

    // Time constants
    int attackSamples = std::max(1, static_cast<int>((attackTimeMs / 1000.0f) * currentSampleRate));
    v.attackIncrement = 1.0f / static_cast<float>(attackSamples);
    v.decayMultiplier = static_cast<float>(std::exp(-1.0 / (currentSampleRate * (decayTimeMs / 1000.0f))));
}

void SampleSlot::stop()
{
    for (auto& v : voices)
    {
        v.active = false;
        v.stage = Voice::EnvStage::Idle;
    }
}

void SampleSlot::render(juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples, bool anySoloActive)
{
    if (!isEnabled || isMute)
    {
        peakMeter.store(0.0f, std::memory_order_relaxed);
        return;
    }

    if (anySoloActive && !isSolo)
    {
        peakMeter.store(0.0f, std::memory_order_relaxed);
        return;
    }

    const juce::ScopedLock sl(bufferLock);
    const int totalSampleLen = sampleBuffer.getNumSamples();
    if (totalSampleLen == 0)
    {
        peakMeter.store(0.0f, std::memory_order_relaxed);
        return;
    }

    const int numOutChannels = outputBuffer.getNumChannels();
    float* outL = outputBuffer.getWritePointer(0);
    float* outR = (numOutChannels > 1) ? outputBuffer.getWritePointer(1) : nullptr;

    const float* srcL = sampleBuffer.getReadPointer(0);
    const float* srcR = (sampleBuffer.getNumChannels() > 1) ? sampleBuffer.getReadPointer(1) : srcL;

    const float phaseFactor = phaseInvert ? -1.0f : 1.0f;
    const float slotGainL = gainLinear * panL * phaseFactor;
    const float slotGainR = gainLinear * panR * phaseFactor;

    float currentMaxPeak = 0.0f;

    for (int s = 0; s < numSamples; ++s)
    {
        int destIdx = startSample + s;
        float accL = 0.0f;
        float accR = 0.0f;

        for (auto& v : voices)
        {
            if (!v.active)
                continue;

            // Envelope progression
            if (v.stage == Voice::EnvStage::Attack)
            {
                v.envLevel += v.attackIncrement;
                if (v.envLevel >= 1.0f)
                {
                    v.envLevel = 1.0f;
                    v.stage = Voice::EnvStage::Decay;
                }
            }
            else if (v.stage == Voice::EnvStage::Decay)
            {
                v.envLevel *= v.decayMultiplier;
                if (v.envLevel <= 0.0005f)
                {
                    v.active = false;
                    v.stage = Voice::EnvStage::Idle;
                    continue;
                }
            }

            // Interpolated sample reading
            int pos0 = static_cast<int>(v.currentPosition);
            if (pos0 >= totalSampleLen - 1)
            {
                v.active = false;
                v.stage = Voice::EnvStage::Idle;
                continue;
            }

            float frac = static_cast<float>(v.currentPosition - pos0);
            int pos1 = pos0 + 1;

            float sampL = srcL[pos0] + frac * (srcL[pos1] - srcL[pos0]);
            float sampR = srcR[pos0] + frac * (srcR[pos1] - srcR[pos0]);

            float voiceGain = v.currentVelocity * v.envLevel;
            accL += sampL * voiceGain;
            accR += sampR * voiceGain;

            v.currentPosition += pitchRatio;
        }

        float mixedL = accL * slotGainL;
        float mixedR = accR * slotGainR;

        outL[destIdx] += mixedL;
        if (outR != nullptr)
            outR[destIdx] += mixedR;

        float p = std::max(std::abs(mixedL), std::abs(mixedR));
        if (p > currentMaxPeak)
            currentMaxPeak = p;
    }

    peakMeter.store(currentMaxPeak, std::memory_order_relaxed);
}

void SampleSlot::getWaveformThumbnail(std::vector<float>& points, int numPoints) const
{
    const juce::ScopedLock sl(bufferLock);
    points.assign(numPoints, 0.0f);
    int len = sampleBuffer.getNumSamples();
    if (len == 0) return;

    const float* src = sampleBuffer.getReadPointer(0);
    int step = std::max(1, len / numPoints);

    for (int i = 0; i < numPoints; ++i)
    {
        int start = i * step;
        int end = std::min(len, start + step);
        float peak = 0.0f;
        for (int j = start; j < end; ++j)
        {
            float a = std::abs(src[j]);
            if (a > peak) peak = a;
        }
        points[i] = juce::jlimit(0.0f, 1.0f, peak);
    }
}

} // namespace yeahwaah
