#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace yeahwaah
{

DrmtrgrrAudioProcessorEditor::DrmtrgrrAudioProcessorEditor(DrmtrgrrAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p),
      headerBar(p),
      visualizer(p.getVisualizerBuffer(), p.getAPVTS().getParameter("detail")),
      detectorPanel(p.getAPVTS()),
      slotRack(p)
{
    juce::LookAndFeel::setDefaultLookAndFeel(&slateLookAndFeel);

    addAndMakeVisible(headerBar);
    addAndMakeVisible(visualizer);
    addAndMakeVisible(detectorPanel);
    addAndMakeVisible(slotRack);

    setSize(1040, 720);
    setResizable(true, true);
    setResizeLimits(960, 680, 1600, 1100);

    startTimerHz(35); // Smooth 35 FPS UI animation
}

DrmtrgrrAudioProcessorEditor::~DrmtrgrrAudioProcessorEditor()
{
    stopTimer();
    juce::LookAndFeel::setDefaultLookAndFeel(nullptr);
}

void DrmtrgrrAudioProcessorEditor::timerCallback()
{
    bool auditioning = *audioProcessor.getAPVTS().getRawParameterValue("auditionFilter") > 0.5f;
    visualizer.setAuditionMode(auditioning);

    visualizer.updateWaveform();
    slotRack.updateMeters();
    headerBar.updateMeters(audioProcessor.getInputPeak(), audioProcessor.getOutputPeak());
}

void DrmtrgrrAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(SlateLookAndFeel::bgDark);
}

void DrmtrgrrAudioProcessorEditor::resized()
{
    auto area = getLocalBounds();

    const int headerHeight = 60;
    headerBar.setBounds(area.removeFromTop(headerHeight));

    const int visualizerHeight = 185;
    visualizer.setBounds(area.removeFromTop(visualizerHeight));

    const int detectorHeight = 105;
    detectorPanel.setBounds(area.removeFromTop(detectorHeight));

    slotRack.setBounds(area);
}

} // namespace yeahwaah
