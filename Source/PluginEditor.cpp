#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "UI/HardwareControls.h"

namespace yeahwaah
{

void DrmtrgrrAudioProcessorEditor::Content::paint(juce::Graphics& g)
{
    hw::drawBrushedMetal(g, getLocalBounds());

    for (auto c : { juce::Point<float>(28.0f, 28.0f), juce::Point<float>(1508.0f, 28.0f),
                    juce::Point<float>(28.0f, 996.0f), juce::Point<float>(1508.0f, 996.0f) })
        hw::drawScrew(g, c, 9.0f, c.x * 0.013f);
}

DrmtrgrrAudioProcessorEditor::DrmtrgrrAudioProcessorEditor(DrmtrgrrAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p),
      headerBar(p),
      visualizer(p.getVisualizerBuffer(),
                 p.getAPVTS().getParameter("detail"),
                 p.getAPVTS().getParameter("lowCut"),
                 p.getAPVTS().getParameter("highCut"),
                 p.getAPVTS().getParameter("retrigger")),
      detectorPanel(p.getAPVTS()),
      browserView(p),
      settingsView(p),
      slotRack(p)
{
    juce::LookAndFeel::setDefaultLookAndFeel(&slateLookAndFeel);

    addAndMakeVisible(content);
    content.setSize(hw::kDesignWidth, hw::kDesignHeight);

    content.addAndMakeVisible(headerBar);
    content.addAndMakeVisible(detectorPanel);
    content.addAndMakeVisible(visualizer);
    content.addChildComponent(browserView);
    content.addChildComponent(settingsView);
    content.addAndMakeVisible(slotRack);

    headerBar.setBounds(0, 0, hw::kDesignWidth, 128);
    detectorPanel.setBounds(0, DetectorPanel::kOriginY, hw::kDesignWidth, 490);
    visualizer.setBounds(DetectorPanel::getScopeBounds());
    browserView.setBounds(30, 134, 1476, 478);
    settingsView.setBounds(30, 134, 1476, 478);
    slotRack.setBounds(0, SlotRackComponent::kOriginY, hw::kDesignWidth, 380);

    // Dropping a sample on the main visualizer loads into Slot 1
    visualizer.onSampleDropped = [this](const juce::File& file) {
        if (audioProcessor.getSlot(0).loadFromFile(file, audioProcessor.getFormatManager()))
            slotRack.updateMeters();
    };

    headerBar.onTabChanged = [this](ActiveTab tab) { updateActiveTabView(tab); };
    updateActiveTabView(ActiveTab::Trigger);

    setResizable(true, false);
    setConstrainer(&constrainer);
    constrainer.setFixedAspectRatio((double) hw::kDesignWidth / hw::kDesignHeight);
    constrainer.setSizeLimits(768, 512, 1920, 1280);
    setSize(1152, 768);

    startTimerHz(35);
}

DrmtrgrrAudioProcessorEditor::~DrmtrgrrAudioProcessorEditor()
{
    stopTimer();
    setConstrainer(nullptr);
    juce::LookAndFeel::setDefaultLookAndFeel(nullptr);
}

void DrmtrgrrAudioProcessorEditor::updateActiveTabView(ActiveTab tab)
{
    const bool trigger = tab == ActiveTab::Trigger;
    visualizer.setVisible(trigger);
    detectorPanel.setVisible(trigger);
    browserView.setVisible(tab == ActiveTab::Browser);
    settingsView.setVisible(tab == ActiveTab::Settings);
}

void DrmtrgrrAudioProcessorEditor::timerCallback()
{
    const bool auditioning = *audioProcessor.getAPVTS().getRawParameterValue("auditionFilter") > 0.5f;
    visualizer.setAuditionMode(auditioning);

    if (visualizer.isVisible())
    {
        visualizer.updateWaveform();
        detectorPanel.setActiveHits(visualizer.getActiveHitCount());
    }

    const float inPeak = audioProcessor.getInputPeak();
    const float outPeak = audioProcessor.getOutputPeak();
    slotRack.updateMeters();
    headerBar.updateMeters(inPeak, outPeak);
    detectorPanel.setMeters(inPeak, outPeak);
}

void DrmtrgrrAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);
}

void DrmtrgrrAudioProcessorEditor::resized()
{
    const float scale = (float) getWidth() / (float) hw::kDesignWidth;
    content.setTransform(juce::AffineTransform::scale(scale));
}

} // namespace yeahwaah
