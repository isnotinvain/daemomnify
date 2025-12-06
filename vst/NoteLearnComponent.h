#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_events/juce_events.h>
#include <functional>

class NoteLearnComponent : public juce::Component, private juce::AsyncUpdater {
   public:
    NoteLearnComponent();

    void setCaption(const juce::String& caption);
    void setLearnedNote(int midiNote);
    int getLearnedNote() const;

    void processNextMidiBuffer(const juce::MidiBuffer& buffer);

    std::function<void(int)> onNoteChanged;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent& event) override;

   private:
    void handleAsyncUpdate() override;
    static juce::String noteNumberToName(int noteNumber);

    juce::String caption{"Note"};
    std::atomic<int> learnedNote{-1};
    std::atomic<bool> isLearning{false};

    juce::Rectangle<int> boxBounds;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NoteLearnComponent)
};
