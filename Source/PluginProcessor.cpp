#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace yeahwaah
{

const juce::StringArray DrmtrgrrAudioProcessor::presetNames = {
    "Rock Kick Puncher",
    "Modern Snare Crack",
    "Fat Snare 80s",
    "Metal Tom Thumper",
    "Acoustic Natural Kit",
    "Ghost Note Hunter"
};

DrmtrgrrAudioProcessor::DrmtrgrrAudioProcessor()
    : AudioProcessor(BusesProperties()
                        .withInput("Input", juce::AudioChannelSet::stereo(), true)
                        .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "Parameters", createParameterLayout()),
      visualizer(8192)
{
    formatManager.registerBasicFormats();

    // Assign slot indices and initialize factory drums
    for (int i = 0; i < NumSlots; ++i)
    {
        slots[i].setSlotIndex(i);
    }
}

DrmtrgrrAudioProcessor::~DrmtrgrrAudioProcessor() = default;

juce::AudioProcessorValueTreeState::ParameterLayout DrmtrgrrAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // Detection & Engine
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "inputGain", "Input Gain", juce::NormalisableRange<float>(-24.0f, 24.0f, 0.1f), 0.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "detail", "Detail", juce::NormalisableRange<float>(-60.0f, 0.0f, 0.1f), -24.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "sensitivity", "Sensitivity", juce::NormalisableRange<float>(0.0f, 100.0f, 0.5f), 80.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "retrigger", "ReTrigger", juce::NormalisableRange<float>(5.0f, 500.0f, 1.0f), 35.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "dynamics", "Dynamics", juce::NormalisableRange<float>(0.0f, 100.0f, 0.5f), 75.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "lowCut", "Low-Cut", juce::NormalisableRange<float>(20.0f, 2000.0f, 1.0f, 0.35f), 60.0f,
        juce::String(), juce::AudioProcessorParameter::genericParameter,
        [](float val, int) { return juce::String(static_cast<int>(val)) + " Hz"; }));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "highCut", "High-Cut", juce::NormalisableRange<float>(200.0f, 20000.0f, 1.0f, 0.35f), 4500.0f,
        juce::String(), juce::AudioProcessorParameter::genericParameter,
        [](float val, int) {
            if (val >= 1000.0f)
                return juce::String(val / 1000.0f, 1) + " kHz";
            return juce::String(static_cast<int>(val)) + " Hz";
        }));

    params.push_back(std::make_unique<juce::AudioParameterBool>(
        "auditionFilter", "Listen Filter", false));

    params.push_back(std::make_unique<juce::AudioParameterBool>(
        "leakageEnable", "Leakage Suppression", false));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "leakageAmount", "Suppression Amount", juce::NormalisableRange<float>(0.0f, 100.0f, 0.5f), 50.0f));

    // Gate
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        "gateEnable", "Gate Enable", false));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "gateThreshold", "Gate Threshold", juce::NormalisableRange<float>(-60.0f, 0.0f, 0.1f), -30.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "gateAttack", "Gate Attack", juce::NormalisableRange<float>(0.1f, 50.0f, 0.1f), 1.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "gateHold", "Gate Hold", juce::NormalisableRange<float>(1.0f, 500.0f, 1.0f), 40.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "gateRelease", "Gate Release", juce::NormalisableRange<float>(5.0f, 1000.0f, 1.0f), 100.0f));

    // MIDI
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        "midiOutEnable", "MIDI Out", true));

    params.push_back(std::make_unique<juce::AudioParameterInt>(
        "midiNote", "MIDI Note", 0, 127, 38));

    params.push_back(std::make_unique<juce::AudioParameterInt>(
        "midiChannel", "MIDI Channel", 1, 16, 10));

    // Master
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "mix", "Mix", juce::NormalisableRange<float>(0.0f, 100.0f, 0.5f), 100.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "outputGain", "Output Gain", juce::NormalisableRange<float>(-24.0f, 12.0f, 0.1f), 0.0f));

    // Per-Slot Parameters
    for (int i = 0; i < NumSlots; ++i)
    {
        juce::String prefix = "slot_" + juce::String(i) + "_";
        juce::String namePrefix = "Slot " + juce::String(i + 1) + " ";

        params.push_back(std::make_unique<juce::AudioParameterBool>(
            prefix + "enable", namePrefix + "Enable", true));

        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            prefix + "volume", namePrefix + "Volume", juce::NormalisableRange<float>(-60.0f, 12.0f, 0.1f), 0.0f,
            juce::String(), juce::AudioProcessorParameter::genericParameter,
            [](float val, int) {
                if (val <= -59.5f) return juce::String("-inf dB");
                return juce::String(val, 1) + " dB";
            }));

        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            prefix + "pan", namePrefix + "Pan", juce::NormalisableRange<float>(-1.0f, 1.0f, 0.01f), 0.0f,
            juce::String(), juce::AudioProcessorParameter::genericParameter,
            [](float val, int) {
                int pct = static_cast<int>(std::round(val * 100.0f));
                if (pct == 0) return juce::String("C");
                if (pct < 0) return juce::String(-pct) + "L";
                return juce::String(pct) + "R";
            }));

        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            prefix + "tune", namePrefix + "Tune", juce::NormalisableRange<float>(-24.0f, 24.0f, 0.1f), 0.0f,
            juce::String(), juce::AudioProcessorParameter::genericParameter,
            [](float val, int) {
                return juce::String(val, 1) + " st";
            }));

        params.push_back(std::make_unique<juce::AudioParameterBool>(
            prefix + "phase", namePrefix + "Phase Invert", false));

        params.push_back(std::make_unique<juce::AudioParameterBool>(
            prefix + "solo", namePrefix + "Solo", false));

        params.push_back(std::make_unique<juce::AudioParameterBool>(
            prefix + "mute", namePrefix + "Mute", false));

        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            prefix + "minVel", namePrefix + "Min Velocity", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.0f));

        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            prefix + "maxVel", namePrefix + "Max Velocity", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 1.0f));

        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            prefix + "attack", namePrefix + "Attack", juce::NormalisableRange<float>(0.1f, 50.0f, 0.1f), 0.5f));

        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            prefix + "decay", namePrefix + "Decay", juce::NormalisableRange<float>(10.0f, 2000.0f, 1.0f), 350.0f));
    }

    return { params.begin(), params.end() };
}

void DrmtrgrrAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    detector.prepare(sampleRate, samplesPerBlock);
    gate.prepare(sampleRate);

    for (int i = 0; i < NumSlots; ++i)
    {
        slots[i].prepare(sampleRate, samplesPerBlock);
    }

    dryCopyBuffer.setSize(2, samplesPerBlock);
    auditionBuffer.setSize(2, samplesPerBlock);
    wetSampleBuffer.setSize(2, samplesPerBlock);

    pendingNoteOffs.clear();
    masterOutputPeak.store(0.0f, std::memory_order_relaxed);
}

void DrmtrgrrAudioProcessor::releaseResources()
{
    for (auto& s : slots)
        s.reset();
    detector.reset();
    gate.reset();
}

bool DrmtrgrrAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    if (layouts.getMainInputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainInputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    return true;
}

void DrmtrgrrAudioProcessor::updateDSPParameters()
{
    detector.setInputGain(*apvts.getRawParameterValue("inputGain"));
    detector.setDetail(*apvts.getRawParameterValue("detail"));
    detector.setSensitivity(*apvts.getRawParameterValue("sensitivity"));
    detector.setRetriggerMs(*apvts.getRawParameterValue("retrigger"));
    detector.setDynamics(*apvts.getRawParameterValue("dynamics"));
    detector.setFilterFrequencies(*apvts.getRawParameterValue("lowCut"), *apvts.getRawParameterValue("highCut"));
    detector.setAuditionFilter(*apvts.getRawParameterValue("auditionFilter") > 0.5f);
    detector.setLeakageSuppression(*apvts.getRawParameterValue("leakageEnable") > 0.5f,
                                  *apvts.getRawParameterValue("leakageAmount"));

    gate.setParameters(*apvts.getRawParameterValue("gateEnable") > 0.5f,
                       *apvts.getRawParameterValue("gateThreshold"),
                       *apvts.getRawParameterValue("gateAttack"),
                       *apvts.getRawParameterValue("gateHold"),
                       *apvts.getRawParameterValue("gateRelease"));

    for (int i = 0; i < NumSlots; ++i)
    {
        juce::String prefix = "slot_" + juce::String(i) + "_";
        slots[i].setEnabled(*apvts.getRawParameterValue(prefix + "enable") > 0.5f);
        slots[i].setVolume(*apvts.getRawParameterValue(prefix + "volume"));
        slots[i].setPan(*apvts.getRawParameterValue(prefix + "pan"));
        slots[i].setTune(*apvts.getRawParameterValue(prefix + "tune"));
        slots[i].setPhaseInvert(*apvts.getRawParameterValue(prefix + "phase") > 0.5f);
        slots[i].setSolo(*apvts.getRawParameterValue(prefix + "solo") > 0.5f);
        slots[i].setMute(*apvts.getRawParameterValue(prefix + "mute") > 0.5f);
        slots[i].setVelocityRange(*apvts.getRawParameterValue(prefix + "minVel"),
                                 *apvts.getRawParameterValue(prefix + "maxVel"));
        slots[i].setEnvelope(*apvts.getRawParameterValue(prefix + "attack"),
                             *apvts.getRawParameterValue(prefix + "decay"));
    }
}

void DrmtrgrrAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();
    if (numSamples == 0) return;

    updateDSPParameters();

    // 1. Maintain buffer sizes
    if (dryCopyBuffer.getNumSamples() < numSamples)
    {
        dryCopyBuffer.setSize(2, numSamples, false, true, true);
        auditionBuffer.setSize(2, numSamples, false, true, true);
        wetSampleBuffer.setSize(2, numSamples, false, true, true);
    }

    // Copy original input for dry path
    for (int ch = 0; ch < std::min(2, numChannels); ++ch)
        dryCopyBuffer.copyFrom(ch, 0, buffer, ch, 0, numSamples);
    if (numChannels == 1)
        dryCopyBuffer.copyFrom(1, 0, buffer, 0, 0, numSamples);

    auditionBuffer.clear();
    wetSampleBuffer.clear();
    detectedTriggers.clear();

    // 2. Run Transient Detection
    detector.process(buffer, auditionBuffer, detectedTriggers, visualizer);

    // If Audition mode is active, output filtered signal directly
    if (detector.isAuditioning())
    {
        for (int ch = 0; ch < numChannels; ++ch)
            buffer.copyFrom(ch, 0, auditionBuffer, std::min(ch, 1), 0, numSamples);

        masterOutputPeak.store(detector.getCurrentPeak(), std::memory_order_relaxed);
        return;
    }

    // 3. Handle incoming MIDI note triggers (MIDI In)
    for (const auto metadata : midiMessages)
    {
        auto msg = metadata.getMessage();
        if (msg.isNoteOn())
        {
            float vel = msg.getFloatVelocity();
            detectedTriggers.push_back({ metadata.samplePosition, vel });
            visualizer.pushHit(vel);
        }
    }

    // 4. Fire sample triggers and generate MIDI Out
    bool midiOut = *apvts.getRawParameterValue("midiOutEnable") > 0.5f;
    int midiNote = static_cast<int>(*apvts.getRawParameterValue("midiNote"));
    int midiChannel = static_cast<int>(*apvts.getRawParameterValue("midiChannel"));

    const int noteDurationSamples = static_cast<int>(getSampleRate() * 0.06); // 60ms note length

    for (const auto& trig : detectedTriggers)
    {
        // Trigger all active sample slots
        for (int i = 0; i < NumSlots; ++i)
        {
            slots[i].trigger(trig.velocity, trig.sampleOffset);
        }

        // Send MIDI Note On
        if (midiOut)
        {
            auto noteOn = juce::MidiMessage::noteOn(midiChannel, midiNote, trig.velocity);
            midiMessages.addEvent(noteOn, trig.sampleOffset);
            pendingNoteOffs.push_back({ midiNote, midiChannel, noteDurationSamples });
        }
    }

    // 5. Render active sample slots into wetSampleBuffer
    bool anySolo = false;
    for (int i = 0; i < NumSlots; ++i)
    {
        if (slots[i].getSolo())
        {
            anySolo = true;
            break;
        }
    }

    for (int i = 0; i < NumSlots; ++i)
    {
        slots[i].render(wetSampleBuffer, 0, numSamples, anySolo);
    }

    // 6. Apply Transient-Driven Gate to Dry audio
    gate.process(dryCopyBuffer);

    // 7. Dry / Wet Blend & Master Output Gain
    float mixPct = *apvts.getRawParameterValue("mix") / 100.0f;
    float dryGain = 1.0f - mixPct;
    float wetGain = mixPct;

    float outGainDb = *apvts.getRawParameterValue("outputGain");
    float masterGain = juce::Decibels::decibelsToGain(outGainDb);

    float* outL = buffer.getWritePointer(0);
    float* outR = (numChannels > 1) ? buffer.getWritePointer(1) : nullptr;

    const float* dryL = dryCopyBuffer.getReadPointer(0);
    const float* dryR = dryCopyBuffer.getReadPointer(1);
    const float* wetL = wetSampleBuffer.getReadPointer(0);
    const float* wetR = wetSampleBuffer.getReadPointer(1);

    float maxOutPeak = 0.0f;

    for (int i = 0; i < numSamples; ++i)
    {
        float blendedL = (dryL[i] * dryGain + wetL[i] * wetGain) * masterGain;
        float blendedR = (dryR[i] * dryGain + wetR[i] * wetGain) * masterGain;

        // Studio soft limiter/saturation to prevent digital clipping
        blendedL = std::tanh(blendedL * 0.95f) * 1.05f;
        blendedR = std::tanh(blendedR * 0.95f) * 1.05f;

        outL[i] = blendedL;
        if (outR != nullptr)
            outR[i] = blendedR;

        float p = std::max(std::abs(blendedL), std::abs(blendedR));
        if (p > maxOutPeak)
            maxOutPeak = p;
    }

    masterOutputPeak.store(maxOutPeak, std::memory_order_relaxed);

    // 8. Process scheduled MIDI Note Off events
    for (auto it = pendingNoteOffs.begin(); it != pendingNoteOffs.end();)
    {
        it->samplesRemaining -= numSamples;
        if (it->samplesRemaining <= 0)
        {
            if (midiOut)
            {
                auto noteOff = juce::MidiMessage::noteOff(it->channel, it->noteNumber, 0.0f);
                midiMessages.addEvent(noteOff, std::max(0, numSamples - 1));
            }
            it = pendingNoteOffs.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

void DrmtrgrrAudioProcessor::triggerSlotManually(int slotIndex, float velocity)
{
    if (slotIndex >= 0 && slotIndex < NumSlots)
    {
        slots[slotIndex].trigger(velocity, 0);
        visualizer.pushHit(velocity);
    }
}

void DrmtrgrrAudioProcessor::applyFactoryPreset(int index)
{
    if (index < 0 || index >= presetNames.size())
        return;

    currentProgramIndex = index;

    switch (index)
    {
        case 0: // Rock Kick Puncher
            *apvts.getRawParameterValue("detail") = -22.0f;
            *apvts.getRawParameterValue("sensitivity") = 85.0f;
            *apvts.getRawParameterValue("retrigger") = 45.0f;
            *apvts.getRawParameterValue("lowCut") = 35.0f;
            *apvts.getRawParameterValue("highCut") = 2500.0f;
            *apvts.getRawParameterValue("midiNote") = 36; // C1
            *apvts.getRawParameterValue("mix") = 100.0f;
            break;

        case 1: // Modern Snare Crack
            *apvts.getRawParameterValue("detail") = -25.0f;
            *apvts.getRawParameterValue("sensitivity") = 88.0f;
            *apvts.getRawParameterValue("retrigger") = 30.0f;
            *apvts.getRawParameterValue("lowCut") = 120.0f;
            *apvts.getRawParameterValue("highCut") = 8000.0f;
            *apvts.getRawParameterValue("midiNote") = 38; // D1
            *apvts.getRawParameterValue("mix") = 100.0f;
            break;

        case 2: // Fat Snare 80s
            *apvts.getRawParameterValue("detail") = -28.0f;
            *apvts.getRawParameterValue("sensitivity") = 78.0f;
            *apvts.getRawParameterValue("retrigger") = 35.0f;
            *apvts.getRawParameterValue("lowCut") = 90.0f;
            *apvts.getRawParameterValue("highCut") = 6500.0f;
            *apvts.getRawParameterValue("midiNote") = 38; // D1
            *apvts.getRawParameterValue("mix") = 90.0f;
            break;

        case 3: // Metal Tom Thumper
            *apvts.getRawParameterValue("detail") = -18.0f;
            *apvts.getRawParameterValue("sensitivity") = 90.0f;
            *apvts.getRawParameterValue("retrigger") = 50.0f;
            *apvts.getRawParameterValue("lowCut") = 70.0f;
            *apvts.getRawParameterValue("highCut") = 3500.0f;
            *apvts.getRawParameterValue("leakageEnable") = 1.0f;
            *apvts.getRawParameterValue("leakageAmount") = 65.0f;
            *apvts.getRawParameterValue("midiNote") = 45; // A1
            *apvts.getRawParameterValue("mix") = 100.0f;
            break;

        case 4: // Acoustic Natural Kit
            *apvts.getRawParameterValue("detail") = -32.0f;
            *apvts.getRawParameterValue("sensitivity") = 70.0f;
            *apvts.getRawParameterValue("retrigger") = 25.0f;
            *apvts.getRawParameterValue("gateEnable") = 1.0f;
            *apvts.getRawParameterValue("gateThreshold") = -32.0f;
            *apvts.getRawParameterValue("mix") = 50.0f; // 50/50 blend
            break;

        case 5: // Ghost Note Hunter
            *apvts.getRawParameterValue("detail") = -38.0f;
            *apvts.getRawParameterValue("sensitivity") = 95.0f;
            *apvts.getRawParameterValue("retrigger") = 18.0f;
            *apvts.getRawParameterValue("dynamics") = 90.0f;
            *apvts.getRawParameterValue("mix") = 100.0f;
            break;
    }
}

juce::AudioProcessorEditor* DrmtrgrrAudioProcessor::createEditor()
{
    return new DrmtrgrrAudioProcessorEditor(*this);
}

bool DrmtrgrrAudioProcessor::hasEditor() const { return true; }
const juce::String DrmtrgrrAudioProcessor::getName() const { return "DRMTRGRR"; }
bool DrmtrgrrAudioProcessor::acceptsMidi() const { return true; }
bool DrmtrgrrAudioProcessor::producesMidi() const { return true; }
bool DrmtrgrrAudioProcessor::isMidiEffect() const { return false; }
double DrmtrgrrAudioProcessor::getTailLengthSeconds() const { return 0.5; }

int DrmtrgrrAudioProcessor::getNumPrograms() { return presetNames.size(); }
int DrmtrgrrAudioProcessor::getCurrentProgram() { return currentProgramIndex; }
void DrmtrgrrAudioProcessor::setCurrentProgram(int index) { applyFactoryPreset(index); }
const juce::String DrmtrgrrAudioProcessor::getProgramName(int index)
{
    if (index >= 0 && index < presetNames.size())
        return presetNames[index];
    return {};
}
void DrmtrgrrAudioProcessor::changeProgramName(int /*index*/, const juce::String& /*newName*/) {}

void DrmtrgrrAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void DrmtrgrrAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState.get() != nullptr && xmlState->hasTagName(apvts.state.getType()))
    {
        apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
    }
}

} // namespace yeahwaah

// JUCE Plugin Creation Entrypoint
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new yeahwaah::DrmtrgrrAudioProcessor();
}
