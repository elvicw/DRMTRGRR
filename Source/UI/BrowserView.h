#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include "../PluginProcessor.h"

namespace yeahwaah
{

class BrowserView : public juce::Component,
                    public juce::FileBrowserListener
{
public:
    BrowserView(DrmtrgrrAudioProcessor& processor);
    ~BrowserView() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    // FileBrowserListener callbacks
    void selectionChanged() override;
    void fileClicked(const juce::File& file, const juce::MouseEvent& e) override;
    void fileDoubleClicked(const juce::File& file) override;
    void browserRootChanged(const juce::File& newRoot) override;

    void auditionFile(const juce::File& file);
    void stopAudition();

private:
    DrmtrgrrAudioProcessor& processor;
    juce::WildcardFileFilter fileFilter;
    juce::TimeSliceThread directoryThread { "FileBrowserThread" };
    juce::DirectoryContentsList directoryContentsList;
    std::unique_ptr<juce::FileTreeComponent> fileTree;

    juce::Label previewTitleLabel;
    juce::TextButton auditionButton { "▶ AUDITION" };
    juce::TextButton loadToSlot1Button { "LOAD INTO CHAN 1" };
    juce::TextButton loadToSlot2Button { "LOAD INTO CHAN 2" };

    juce::AudioTransportSource transportSource;
    std::unique_ptr<juce::AudioFormatReaderSource> currentAudioSource;

    juce::File currentlySelectedFile;
    std::vector<float> previewWaveform;

    void loadSelectedFileIntoSlot(int slotIndex);
    void extractPreviewWaveform(const juce::File& file);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BrowserView)
};

} // namespace yeahwaah
