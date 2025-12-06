#include "NoteLearnComponent.h"

NoteLearnComponent::NoteLearnComponent() {}

void NoteLearnComponent::setCaption(const juce::String& newCaption) {
    caption = newCaption;
    repaint();
}

void NoteLearnComponent::setLearnedNote(int midiNote) {
    learnedNote.store(midiNote);
    repaint();
}

int NoteLearnComponent::getLearnedNote() const { return learnedNote.load(); }

void NoteLearnComponent::processNextMidiBuffer(const juce::MidiBuffer& buffer) {
    if (!isLearning.load())
        return;

    for (const auto& metadata : buffer) {
        auto msg = metadata.getMessage();
        if (msg.isNoteOn()) {
            learnedNote.store(msg.getNoteNumber());
            isLearning.store(false);
            triggerAsyncUpdate();
            if (onNoteChanged)
                onNoteChanged(msg.getNoteNumber());
            return;
        }
    }
}

void NoteLearnComponent::paint(juce::Graphics& g) {
    auto bounds = getLocalBounds();

    g.setColour(juce::Colours::white);
    auto font = g.getCurrentFont();
    auto captionWidth = font.getStringWidth(caption + ": ");

    auto captionBounds = bounds.removeFromLeft(captionWidth + 5);
    g.drawText(caption + ":", captionBounds, juce::Justification::centredRight);

    boxBounds = bounds.reduced(2);

    if (isLearning.load()) {
        g.setColour(juce::Colours::yellow.withAlpha(0.3f));
    } else {
        g.setColour(juce::Colours::black);
    }
    g.fillRect(boxBounds);

    g.setColour(isLearning.load() ? juce::Colours::yellow : juce::Colours::grey);
    g.drawRect(boxBounds, 1);

    int note = learnedNote.load();
    juce::String displayText;
    if (isLearning.load()) {
        displayText = "...";
    } else if (note >= 0) {
        displayText = noteNumberToName(note);
    }

    g.setColour(juce::Colours::white);
    g.drawText(displayText, boxBounds, juce::Justification::centred);
}

void NoteLearnComponent::resized() {}

void NoteLearnComponent::mouseDown(const juce::MouseEvent& event) {
    if (boxBounds.contains(event.getPosition())) {
        isLearning.store(true);
        repaint();
    }
}

void NoteLearnComponent::handleAsyncUpdate() {
    repaint();
}

juce::String NoteLearnComponent::noteNumberToName(int noteNumber) {
    if (noteNumber < 0 || noteNumber > 127)
        return {};

    static const char* noteNames[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
    int octave = (noteNumber / 12) - 1;
    int noteIndex = noteNumber % 12;

    return juce::String(noteNames[noteIndex]) + juce::String(octave);
}
