#pragma once
#include <juce_core/juce_core.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <atomic>
#include <vector>

namespace yeahwaah
{

struct HitEvent
{
    float positionNormalized; // 0.0 to 1.0 on visualizer timeline
    float velocity;           // 0.0 to 1.0
    int slotMask;             // bitmask of slots triggered
    int64_t timestampMs;
};

// Thread-safe ring buffer for oscilloscope waveform & hit visualization
class VisualizerBuffer
{
public:
    VisualizerBuffer(int capacity = 8192)
        : bufferSize(capacity), waveform(static_cast<size_t>(capacity), 0.0f)
    {
        writeIndex.store(0, std::memory_order_relaxed);
    }

    void pushSample(float sample)
    {
        int idx = writeIndex.load(std::memory_order_relaxed);
        waveform[static_cast<size_t>(idx)] = sample;
        writeIndex.store((idx + 1) % bufferSize, std::memory_order_release);
    }

    void pushSamples(const float* samples, int numSamples, int decimation = 4)
    {
        int idx = writeIndex.load(std::memory_order_relaxed);
        for (int i = 0; i < numSamples; i += decimation)
        {
            waveform[static_cast<size_t>(idx)] = samples[i];
            idx = (idx + 1) % bufferSize;
        }
        writeIndex.store(idx, std::memory_order_release);
    }

    void pushHit(float velocity, int slotMask = 0xFF)
    {
        const juce::SpinLock::ScopedLockType sl(hitLock);
        recentHits.push_back({ 1.0f, velocity, slotMask, juce::Time::currentTimeMillis() });
        if (recentHits.size() > 64)
            recentHits.erase(recentHits.begin());
    }

    // Copies recent waveform history into destination buffer (chronological order)
    void getWaveform(std::vector<float>& dest) const
    {
        dest.resize(bufferSize);
        int currentWrite = writeIndex.load(std::memory_order_acquire);
        for (size_t i = 0; i < static_cast<size_t>(bufferSize); ++i)
        {
            size_t srcIdx = static_cast<size_t>((currentWrite + static_cast<int>(i)) % bufferSize);
            dest[i] = waveform[srcIdx];
        }
    }

    std::vector<HitEvent> getRecentHits()
    {
        const juce::SpinLock::ScopedLockType sl(hitLock);
        int64_t now = juce::Time::currentTimeMillis();
        // remove hits older than 1500ms
        recentHits.erase(
            std::remove_if(recentHits.begin(), recentHits.end(),
                           [now](const HitEvent& h) { return (now - h.timestampMs) > 1500; }),
            recentHits.end());
        return recentHits;
    }

    int getCapacity() const { return bufferSize; }

private:
    const int bufferSize;
    std::vector<float> waveform;
    std::atomic<int> writeIndex;

    juce::SpinLock hitLock;
    std::vector<HitEvent> recentHits;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VisualizerBuffer)
};

} // namespace yeahwaah
