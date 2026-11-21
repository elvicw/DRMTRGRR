#include "TransientDetector.h"
#include <cmath>

namespace yeahwaah
{

TransientDetector::TransientDetector()
{
    updateFilterCoefficients();
}

void TransientDetector::prepare(double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate > 0.0 ? sampleRate : 44100.0;
    
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = currentSampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32>(samplesPerBlock);
    spec.numChannels = 1;

    lowCutFilterLeft.prepare(spec);
    lowCutFilterRight.prepare(spec);
    highCutFilterLeft.prepare(spec);
    highCutFilterRight.prepare(spec);

    updateFilterCoefficients();
    reset();
}

void TransientDetector::reset()
{
    lowCutFilterLeft.reset();
    lowCutFilterRight.reset();
    highCutFilterLeft.reset();
    highCutFilterRight.reset();

    envFast = 0.0f;
    envSlow = 0.0f;
    envLeakage = 0.0f;
    lockoutSamplesRemaining = 0;
    peakHoldSamplesRemaining = 0;
    peakValueInWindow = 0.0f;
    pendingTriggerSampleOffset = -1;
    currentPeak.store(0.0f, std::memory_order_relaxed);

    // Envelope time constants
    // Fast attack: ~0.3ms
    alphaFast = static_cast<float>(std::exp(-1.0 / (currentSampleRate * 0.0003)));
    // Slow decay: ~30ms
    alphaSlow = static_cast<float>(std::exp(-1.0 / (currentSampleRate * 0.030)));
    // Leakage background: ~150ms
    alphaLeakage = static_cast<float>(std::exp(-1.0 / (currentSampleRate * 0.150)));

    retriggerSamples = static_cast<int>((retriggerMs / 1000.0f) * currentSampleRate);
}

void TransientDetector::updateFilterCoefficients()
{
    float lowCut = juce::jlimit(20.0f, static_cast<float>(currentSampleRate * 0.45), lowCutFreq);
    float highCut = juce::jlimit(lowCut + 20.0f, static_cast<float>(currentSampleRate * 0.49), highCutFreq);

    auto lowPassCoeffs = juce::dsp::IIR::Coefficients<float>::makeLowPass(currentSampleRate, highCut);
    auto highPassCoeffs = juce::dsp::IIR::Coefficients<float>::makeHighPass(currentSampleRate, lowCut);

    lowCutFilterLeft.coefficients = highPassCoeffs;
    lowCutFilterRight.coefficients = highPassCoeffs;
    highCutFilterLeft.coefficients = lowPassCoeffs;
    highCutFilterRight.coefficients = lowPassCoeffs;
}

void TransientDetector::setInputGain(float gainDb)
{
    inputGainLinear = juce::Decibels::decibelsToGain(gainDb);
}

void TransientDetector::setDetail(float dB)
{
    detailDb = juce::jlimit(-60.0f, 0.0f, dB);
    detailLinear = juce::Decibels::decibelsToGain(detailDb);
}

void TransientDetector::setSensitivity(float sensitivityPct)
{
    // 0 to 100% -> internal scaling factor 0.1 to 2.5
    sensitivity = juce::jlimit(0.0f, 100.0f, sensitivityPct) / 100.0f;
}

void TransientDetector::setRetriggerMs(float ms)
{
    retriggerMs = juce::jlimit(5.0f, 500.0f, ms);
    retriggerSamples = static_cast<int>((retriggerMs / 1000.0f) * currentSampleRate);
}

void TransientDetector::setDynamics(float dynamicsPct)
{
    dynamics = juce::jlimit(0.0f, 100.0f, dynamicsPct) / 100.0f;
}

void TransientDetector::setFilterFrequencies(float lowCutHz, float highCutHz)
{
    if (std::abs(lowCutFreq - lowCutHz) > 0.5f || std::abs(highCutFreq - highCutHz) > 0.5f)
    {
        lowCutFreq = lowCutHz;
        highCutFreq = highCutHz;
        updateFilterCoefficients();
    }
}

void TransientDetector::setAuditionFilter(bool audition)
{
    auditionFilter = audition;
}

void TransientDetector::setLeakageSuppression(bool enable, float amountPct)
{
    leakageEnabled = enable;
    leakageAmount = juce::jlimit(0.0f, 100.0f, amountPct) / 100.0f;
}

void TransientDetector::process(const juce::AudioBuffer<float>& inputBuffer,
                                juce::AudioBuffer<float>& auditionBuffer,
                                std::vector<TriggerEvent>& detectedTriggers,
                                VisualizerBuffer& visualizer)
{
    const int numSamples = inputBuffer.getNumSamples();
    const int numChannels = inputBuffer.getNumChannels();
    if (numSamples == 0) return;

    const float* inL = inputBuffer.getReadPointer(0);
    const float* inR = numChannels > 1 ? inputBuffer.getReadPointer(1) : inL;

    float* audL = auditionBuffer.getWritePointer(0);
    float* audR = auditionBuffer.getNumChannels() > 1 ? auditionBuffer.getWritePointer(1) : audL;

    float blockMaxPeak = 0.0f;
    const int peakHoldDuration = static_cast<int>(currentSampleRate * 0.0025); // ~2.5ms peak window

    // Detection multiplier based on sensitivity
    // Sensitivity = 1.0 (100%) means lower threshold needed; Sensitivity = 0.0 means high threshold needed
    const float sensMultiplier = 0.3f + 1.7f * sensitivity;
    const float effectiveThreshold = detailLinear / sensMultiplier;

    for (int i = 0; i < numSamples; ++i)
    {
        // 1. Input gain
        float sampleL = inL[i] * inputGainLinear;
        float sampleR = inR[i] * inputGainLinear;

        // 2. Bandpass filtering (High-pass followed by Low-pass)
        float hpSampleL = lowCutFilterLeft.processSample(sampleL);
        float hpSampleR = lowCutFilterRight.processSample(sampleR);
        float filtSampleL = highCutFilterLeft.processSample(hpSampleL);
        float filtSampleR = highCutFilterRight.processSample(hpSampleR);

        float monoFilt = 0.5f * (filtSampleL + filtSampleR);
        float absFilt = std::abs(monoFilt);

        // Track peak for UI metering
        if (absFilt > blockMaxPeak)
            blockMaxPeak = absFilt;

        // Audition routing: if audition is ON, output filtered audio
        if (auditionFilter)
        {
            audL[i] = filtSampleL;
            audR[i] = filtSampleR;
        }

        // 3. Transient Envelope Extraction
        if (absFilt > envFast)
            envFast = absFilt; // instant attack
        else
            envFast = absFilt + alphaFast * (envFast - absFilt);

        envSlow = absFilt + alphaSlow * (envSlow - absFilt);
        envLeakage = absFilt + alphaLeakage * (envLeakage - absFilt);

        // Differential flux (rate of energy rise)
        float diffFlux = std::max(0.0f, envFast - envSlow);

        // 4. Retrigger & Lockout handling
        if (lockoutSamplesRemaining > 0)
        {
            --lockoutSamplesRemaining;
        }

        // 5. Peak tracking after transient detection
        if (peakHoldSamplesRemaining > 0)
        {
            if (absFilt > peakValueInWindow)
                peakValueInWindow = absFilt;

            --peakHoldSamplesRemaining;

            // When peak hold window completes, fire the final trigger event with accurate velocity
            if (peakHoldSamplesRemaining == 0 && pendingTriggerSampleOffset >= 0)
            {
                // Map peak to velocity (0.0 to 1.0)
                // Normalize by headroom above threshold
                float normalizedPeak = (peakValueInWindow - effectiveThreshold * 0.5f) / (1.0f - effectiveThreshold * 0.5f + 1e-5f);
                normalizedPeak = juce::jlimit(0.05f, 1.0f, normalizedPeak);

                // Apply dynamics response curve (linear to exponential)
                float exponent = 1.6f - 1.2f * dynamics; // dynamics: 0 -> static, 1 -> full dynamic range
                float finalVelocity = std::pow(normalizedPeak, exponent);
                finalVelocity = juce::jlimit(0.08f, 1.0f, finalVelocity);

                detectedTriggers.push_back({ pendingTriggerSampleOffset, finalVelocity });
                visualizer.pushHit(finalVelocity);
                pendingTriggerSampleOffset = -1;
            }
        }

        // 6. Threshold crossing check
        if (lockoutSamplesRemaining == 0 && peakHoldSamplesRemaining == 0)
        {
            bool exceedsThreshold = (diffFlux >= effectiveThreshold) || (absFilt >= effectiveThreshold && envFast > envSlow * 1.5f);

            // Leakage suppression check
            if (exceedsThreshold && leakageEnabled)
            {
                // If background bleed is high, require higher flux ratio
                float bleedThreshold = envLeakage * (1.0f + leakageAmount * 2.5f);
                if (diffFlux < bleedThreshold)
                    exceedsThreshold = false;
            }

            if (exceedsThreshold)
            {
                lockoutSamplesRemaining = retriggerSamples;
                peakHoldSamplesRemaining = peakHoldDuration;
                peakValueInWindow = absFilt;
                pendingTriggerSampleOffset = i;
            }
        }
    }

    currentPeak.store(blockMaxPeak, std::memory_order_relaxed);

    // Push waveform samples into visualizer FIFO (downsampled for smooth rendering)
    if (auditionFilter)
        visualizer.pushSamples(audL, numSamples, 4);
    else
        visualizer.pushSamples(inL, numSamples, 4);
}

} // namespace yeahwaah
