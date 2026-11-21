#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace yeahwaah
{

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

    addAndMakeVisible(headerBar);
    addAndMakeVisible(visualizer);
    addAndMakeVisible(detectorPanel);
    addChildComponent(browserView);
    addChildComponent(settingsView);
    addAndMakeVisible(slotRack);

    // Dropping a sample on the main visualizer loads into Slot 1
    visualizer.onSampleDropped = [this](const juce::File& file) {
        if (audioProcessor.getSlot(0).loadFromFile(file, audioProcessor.getFormatManager()))
        {
            slotRack.updateMeters();
        }
    };

    headerBar.onTabChanged = [this](ActiveTab tab) {
        updateActiveTabView(tab);
    };

    updateActiveTabView(ActiveTab::Trigger);

    setSize(1240, 860);
    setResizable(true, true);
    setResizeLimits(1060, 740, 1800, 1200);

    startTimerHz(35); // Smooth 35 FPS UI animation
}

DrmtrgrrAudioProcessorEditor::~DrmtrgrrAudioProcessorEditor()
{
    stopTimer();
    juce::LookAndFeel::setDefaultLookAndFeel(nullptr);
}

void DrmtrgrrAudioProcessorEditor::updateActiveTabView(ActiveTab tab)
{
    visualizer.setVisible(tab == ActiveTab::Trigger);
    detectorPanel.setVisible(tab == ActiveTab::Trigger);
    browserView.setVisible(tab == ActiveTab::Browser);
    settingsView.setVisible(tab == ActiveTab::Settings);

    resized();
}

void DrmtrgrrAudioProcessorEditor::timerCallback()
{
    bool auditioning = *audioProcessor.getAPVTS().getRawParameterValue("auditionFilter") > 0.5f;
    visualizer.setAuditionMode(auditioning);

    if (visualizer.isVisible())
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

    const int headerHeight = 70;
    headerBar.setBounds(area.removeFromTop(headerHeight));

    const int rackHeight = 410;
    slotRack.setBounds(area.removeFromBottom(rackHeight));

    // Middle area is occupied by the active tab
    if (visualizer.isVisible())
    {
        const int visHeight = 210;
        visualizer.setBounds(area.removeFromTop(visHeight));
        detectorPanel.setBounds(area);
    }
    else if (browserView.isVisible())
    {
        browserView.setBounds(area);
    }
    else if (settingsView.isVisible())
    {
        settingsView.setBounds(area);
    }
}

} // namespace yeahwaah
