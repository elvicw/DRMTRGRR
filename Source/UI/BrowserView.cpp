#include "BrowserView.h"
#include "SlateLookAndFeel.h"

namespace yeahwaah
{

BrowserView::BrowserView(DrmtrgrrAudioProcessor& proc)
    : processor(proc),
      fileFilter("*.wav;*.aif;*.aiff;*.mp3;*.flac", "*", "Audio Files"),
      directoryContentsList(&fileFilter, directoryThread)
{
    directoryThread.startThread();
    directoryContentsList.setDirectory(juce::File::getSpecialLocation(juce::File::userHomeDirectory), true, true);

    fileTree = std::make_unique<juce::FileTreeComponent>(directoryContentsList);
    fileTree->addListener(this);
    addAndMakeVisible(*fileTree);

    previewTitleLabel.setText("SELECT A DRUM SAMPLE TO PREVIEW", juce::dontSendNotification);
    previewTitleLabel.setFont(juce::Font(12.0f, juce::Font::bold));
    previewTitleLabel.setColour(juce::Label::textColourId, SlateLookAndFeel::textLight);
    previewTitleLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(previewTitleLabel);

    auditionButton.onClick = [this] {
        if (currentlySelectedFile.existsAsFile())
            auditionFile(currentlySelectedFile);
    };
    addAndMakeVisible(auditionButton);

    loadToSlot1Button.onClick = [this] { loadSelectedFileIntoSlot(0); };
    addAndMakeVisible(loadToSlot1Button);

    loadToSlot2Button.onClick = [this] { loadSelectedFileIntoSlot(1); };
    addAndMakeVisible(loadToSlot2Button);
}

BrowserView::~BrowserView()
{
    stopAudition();
    fileTree->removeListener(this);
    directoryThread.stopThread(1000);
}

void BrowserView::selectionChanged()
{
    auto selectedFile = fileTree->getSelectedFile();
    if (selectedFile.existsAsFile())
    {
        currentlySelectedFile = selectedFile;
        previewTitleLabel.setText(selectedFile.getFileName(), juce::dontSendNotification);
        extractPreviewWaveform(selectedFile);
        repaint();
    }
}

void BrowserView::fileClicked(const juce::File& file, const juce::MouseEvent& /*e*/)
{
    if (file.existsAsFile())
    {
        currentlySelectedFile = file;
        previewTitleLabel.setText(file.getFileName(), juce::dontSendNotification);
        extractPreviewWaveform(file);
        repaint();
    }
}

void BrowserView::fileDoubleClicked(const juce::File& file)
{
    if (file.existsAsFile())
    {
        currentlySelectedFile = file;
        loadSelectedFileIntoSlot(0);
        auditionFile(file);
    }
}

void BrowserView::browserRootChanged(const juce::File& /*newRoot*/)
{
}

void BrowserView::auditionFile(const juce::File& file)
{
    if (processor.getSlot(0).loadFromFile(file, processor.getFormatManager()))
    {
        processor.triggerSlotManually(0, 1.0f);
    }
}

void BrowserView::stopAudition()
{
}

void BrowserView::loadSelectedFileIntoSlot(int slotIndex)
{
    if (currentlySelectedFile.existsAsFile())
    {
        processor.getSlot(slotIndex).loadFromFile(currentlySelectedFile, processor.getFormatManager());
        processor.triggerSlotManually(slotIndex, 1.0f);
    }
}

void BrowserView::extractPreviewWaveform(const juce::File& file)
{
    previewWaveform.assign(128, 0.0f);
    std::unique_ptr<juce::AudioFormatReader> reader(processor.getFormatManager().createReaderFor(file));
    if (reader == nullptr) return;

    juce::AudioBuffer<float> tempBuffer(1, (int)reader->lengthInSamples);
    reader->read(&tempBuffer, 0, (int)reader->lengthInSamples, 0, true, false);

    const float* src = tempBuffer.getReadPointer(0);
    int len = tempBuffer.getNumSamples();
    int step = std::max(1, len / 128);

    for (int i = 0; i < 128; ++i)
    {
        int start = i * step;
        int end = std::min(len, start + step);
        float peak = 0.0f;
        for (int j = start; j < end; ++j)
        {
            float a = std::abs(src[j]);
            if (a > peak) peak = a;
        }
        previewWaveform[static_cast<size_t>(i)] = peak;
    }
}

void BrowserView::paint(juce::Graphics& g)
{
    const int w = getWidth();
    const int h = getHeight();

    // Dark slate background
    g.setColour(SlateLookAndFeel::bgDark);
    g.fillRect(0, 0, w, h);

    // Right Preview Pane
    const int treeW = 380;
    juce::Rectangle<float> previewBox(treeW + 12.0f, 10.0f, (float)w - treeW - 24.0f, (float)h - 20.0f);

    g.setColour(SlateLookAndFeel::bgPanel);
    g.fillRoundedRectangle(previewBox, 4.0f);
    g.setColour(SlateLookAndFeel::slateBorder);
    g.drawRoundedRectangle(previewBox, 4.0f, 1.0f);

    // Waveform preview display box
    juce::Rectangle<float> waveArea(previewBox.getX() + 16.0f, previewBox.getY() + 38.0f,
                                    previewBox.getWidth() - 32.0f, 120.0f);

    g.setColour(juce::Colour(0xff090c12));
    g.fillRoundedRectangle(waveArea, 3.0f);
    g.setColour(SlateLookAndFeel::slateBorder);
    g.drawRoundedRectangle(waveArea, 3.0f, 1.0f);

    // Render preview waveform
    if (!previewWaveform.empty())
    {
        const int numPts = (int)previewWaveform.size();
        const float step = (waveArea.getWidth() - 4.0f) / static_cast<float>(numPts);
        const float midY = waveArea.getCentreY();

        g.setColour(SlateLookAndFeel::cyanGlow.withAlpha(0.85f));
        for (int i = 0; i < numPts; ++i)
        {
            float peakH = previewWaveform[static_cast<size_t>(i)] * (waveArea.getHeight() * 0.44f);
            float px = waveArea.getX() + 2.0f + static_cast<float>(i) * step;
            g.drawVerticalLine(static_cast<int>(px), midY - peakH, midY + peakH);
        }
    }
}

void BrowserView::resized()
{
    const int w = getWidth();
    const int h = getHeight();
    const int treeW = 380;

    fileTree->setBounds(10, 10, treeW, h - 20);

    const int rightX = treeW + 28;
    const int rightW = w - treeW - 56;

    previewTitleLabel.setBounds(rightX, 16, rightW, 24);

    auditionButton.setBounds(rightX + rightW / 2 - 60, 180, 120, 28);
    loadToSlot1Button.setBounds(rightX, 225, (rightW - 10) / 2, 28);
    loadToSlot2Button.setBounds(rightX + (rightW - 10) / 2 + 10, 225, (rightW - 10) / 2, 28);
}

} // namespace yeahwaah
