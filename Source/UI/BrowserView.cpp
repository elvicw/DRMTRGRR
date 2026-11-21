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
    previewTitleLabel.setFont(juce::Font(12.5f, juce::Font::bold));
    previewTitleLabel.setColour(juce::Label::textColourId, SlateLookAndFeel::textLight);
    previewTitleLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(previewTitleLabel);

    metadataLabel.setText("No file selected", juce::dontSendNotification);
    metadataLabel.setFont(juce::Font(10.0f, juce::Font::plain));
    metadataLabel.setColour(juce::Label::textColourId, SlateLookAndFeel::textDim);
    metadataLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(metadataLabel);

    auditionButton.onClick = [this] {
        if (currentlySelectedFile.existsAsFile())
            auditionFile(currentlySelectedFile);
    };
    addAndMakeVisible(auditionButton);

    autoAuditionToggle.setToggleState(false, juce::dontSendNotification);
    addAndMakeVisible(autoAuditionToggle);

    // Create 6 channel target buttons
    for (int i = 0; i < DrmtrgrrAudioProcessor::NumSlots; ++i)
    {
        auto btn = std::make_unique<juce::TextButton>("LOAD INTO CH " + juce::String(i + 1));
        btn->onClick = [this, i] { loadSelectedFileIntoSlot(i); };
        addAndMakeVisible(*btn);
        loadButtons.push_back(std::move(btn));
    }
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
        if (autoAuditionToggle.getToggleState())
            auditionFile(selectedFile);
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
        if (autoAuditionToggle.getToggleState())
            auditionFile(file);
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
    if (reader == nullptr)
    {
        metadataLabel.setText("Unsupported audio format", juce::dontSendNotification);
        return;
    }

    double lengthSec = static_cast<double>(reader->lengthInSamples) / reader->sampleRate;
    juce::String meta = "Sample Rate: " + juce::String(static_cast<int>(reader->sampleRate)) + " Hz | "
                        + juce::String(reader->numChannels) + " Channels | "
                        + juce::String(reader->bitsPerSample) + "-bit | "
                        + juce::String(lengthSec, 2) + " sec | "
                        + juce::File::descriptionOfSizeInBytes(file.getSize());
    metadataLabel.setText(meta, juce::dontSendNotification);

    juce::AudioBuffer<float> tempBuffer(1, static_cast<int>(reader->lengthInSamples));
    reader->read(&tempBuffer, 0, static_cast<int>(reader->lengthInSamples), 0, true, false);

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
    const int treeW = 360;
    juce::Rectangle<float> previewBox(static_cast<float>(treeW) + 12.0f, 8.0f, static_cast<float>(w) - static_cast<float>(treeW) - 20.0f, static_cast<float>(h) - 16.0f);

    g.setColour(SlateLookAndFeel::bgPanel);
    g.fillRoundedRectangle(previewBox, 4.0f);
    g.setColour(SlateLookAndFeel::slateBorder);
    g.drawRoundedRectangle(previewBox, 4.0f, 1.0f);

    // Waveform preview display box
    juce::Rectangle<float> waveArea(previewBox.getX() + 16.0f, previewBox.getY() + 48.0f,
                                    previewBox.getWidth() - 32.0f, 95.0f);

    g.setColour(juce::Colour(0xff090c12));
    g.fillRoundedRectangle(waveArea, 3.0f);
    g.setColour(SlateLookAndFeel::slateBorder);
    g.drawRoundedRectangle(waveArea, 3.0f, 1.0f);

    // Center guideline
    g.setColour(juce::Colour(0xff161c28));
    g.drawHorizontalLine(static_cast<int>(waveArea.getCentreY()), waveArea.getX() + 2.0f, waveArea.getRight() - 2.0f);

    // Render preview waveform
    if (!previewWaveform.empty())
    {
        const int numPts = static_cast<int>(previewWaveform.size());
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
    const int treeW = 360;

    fileTree->setBounds(8, 8, treeW, h - 16);

    const int rightX = treeW + 28;
    const int rightW = w - treeW - 52;

    previewTitleLabel.setBounds(rightX, 10, rightW, 18);
    metadataLabel.setBounds(rightX, 28, rightW, 16);

    // Audition Button & Auto-Audition checkbox
    auditionButton.setBounds(rightX + 16, 150, 140, 28);
    autoAuditionToggle.setBounds(rightX + 170, 152, 200, 24);

    // Channel routing buttons grid (2 rows of 3 buttons)
    const int btnH = 26;
    const int gap = 8;
    const int colW = (rightW - 32 - gap * 2) / 3;

    for (int i = 0; i < DrmtrgrrAudioProcessor::NumSlots; ++i)
    {
        int row = i / 3;
        int col = i % 3;
        int bx = rightX + 16 + col * (colW + gap);
        int by = 188 + row * (btnH + gap);
        if (i < static_cast<int>(loadButtons.size()))
            loadButtons[static_cast<size_t>(i)]->setBounds(bx, by, colW, btnH);
    }
}

} // namespace yeahwaah
