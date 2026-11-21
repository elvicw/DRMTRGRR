#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include "../Source/PluginProcessor.h"
#include "../Source/PluginEditor.h"

int main()
{
    juce::ScopedJuceInitialiser_GUI gui;
    yeahwaah::DrmtrgrrAudioProcessor proc;

    // Fill in simulated live drum track audio in the visualizer buffer
    for (int i = 0; i < 8192; ++i)
    {
        float phase = static_cast<float>(i) / 8192.0f;
        float hit1 = std::exp(-std::max(0.0f, phase - 0.20f) * 16.0f) * std::sin((phase - 0.20f) * 75.0f);
        float hit2 = std::exp(-std::max(0.0f, phase - 0.55f) * 20.0f) * std::sin((phase - 0.55f) * 85.0f);
        float hit3 = std::exp(-std::max(0.0f, phase - 0.85f) * 24.0f) * std::sin((phase - 0.85f) * 95.0f);
        float noise = ((static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) - 0.5f) * 0.035f;
        float s = (phase >= 0.20f ? hit1 * 0.88f : 0.0f)
                + (phase >= 0.55f ? hit2 * 0.65f : 0.0f)
                + (phase >= 0.85f ? hit3 * 0.95f : 0.0f)
                + noise;
        proc.getVisualizerBuffer().pushSample(s);
    }

    // Push simulated transient hit events
    proc.getVisualizerBuffer().pushHit(0.95f, 0x01);
    proc.getVisualizerBuffer().pushHit(0.72f, 0x02);

    // Load factory drums into slots
    for (int i = 0; i < yeahwaah::DrmtrgrrAudioProcessor::NumSlots; ++i)
    {
        proc.getSlot(i).loadFactoryDrum(static_cast<yeahwaah::FactoryDrumType>(i % static_cast<int>(yeahwaah::FactoryDrumType::Count)));
    }

    // Trigger test hits to illuminate meters and indicators
    proc.triggerSlotManually(0, 0.96f);
    proc.triggerSlotManually(1, 0.84f);
    proc.triggerSlotManually(2, 0.68f);
    proc.triggerSlotManually(3, 0.45f);

    std::unique_ptr<juce::AudioProcessorEditor> editor(proc.createEditorIfNeeded());
    if (!editor)
    {
        std::cerr << "Failed to create editor\n";
        return 1;
    }

    editor->setSize(1240, 860);
    editor->setVisible(true);
    std::cout << "Editor bounds: " << editor->getBounds().toString() << std::endl;

    auto* typedEditor = dynamic_cast<yeahwaah::DrmtrgrrAudioProcessorEditor*>(editor.get());
    if (typedEditor != nullptr)
    {
        typedEditor->timerCallback();
    }

    // High quality 2x Retina snapshot
    auto img = editor->createComponentSnapshot(editor->getLocalBounds(), false, 2.0f);

    juce::File outFile = juce::File::getCurrentWorkingDirectory().getChildFile("assets/screenshot.png");
    outFile.getParentDirectory().createDirectory();
    outFile.deleteFile();

    juce::FileOutputStream fos(outFile);
    if (fos.openedOk())
    {
        juce::PNGImageFormat png;
        if (png.writeImageToStream(img, fos))
        {
            std::cout << "Successfully generated " << outFile.getFullPathName() << std::endl;
            return 0;
        }
    }

    std::cerr << "Failed to write PNG\n";
    return 1;
}
