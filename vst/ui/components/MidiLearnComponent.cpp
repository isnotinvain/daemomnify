#include "MidiLearnComponent.h"

MidiLearnComponent::MidiLearnComponent() {}

void MidiLearnComponent::setCaption(const juce::String& newCaption) {
    caption = newCaption;
    repaint();
}

void MidiLearnComponent::setLearnedValue(MidiLearnedValue val) {
    learnedType.store(val.type);
    learnedValue.store(val.value);
    repaint();
}

MidiLearnedValue MidiLearnComponent::getLearnedValue() const {
    return {learnedType.load(), learnedValue.load()};
}

void MidiLearnComponent::processNextMidiBuffer(const juce::MidiBuffer& buffer) {
    if (!isLearning.load())
        return;

    for (const auto& metadata : buffer) {
        auto msg = metadata.getMessage();
        if (msg.isNoteOn()) {
            learnedType.store(MidiLearnedType::Note);
            learnedValue.store(msg.getNoteNumber());
            isLearning.store(false);
            triggerAsyncUpdate();
            if (onValueChanged)
                onValueChanged({MidiLearnedType::Note, msg.getNoteNumber()});
            return;
        } else if (msg.isController()) {
            learnedType.store(MidiLearnedType::CC);
            learnedValue.store(msg.getControllerNumber());
            isLearning.store(false);
            triggerAsyncUpdate();
            if (onValueChanged)
                onValueChanged({MidiLearnedType::CC, msg.getControllerNumber()});
            return;
        }
    }
}

juce::String MidiLearnComponent::getDisplayText() const {
    if (isLearning.load()) {
        return "...";
    }

    auto type = learnedType.load();
    auto value = learnedValue.load();

    if (type == MidiLearnedType::None || value < 0) {
        return "";
    }

    if (type == MidiLearnedType::Note) {
        return noteNumberToName(value);
    } else {
        return "CC" + juce::String(value);
    }
}

void MidiLearnComponent::paint(juce::Graphics& g) {
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

    g.setColour(juce::Colours::white);
    g.drawText(getDisplayText(), boxBounds, juce::Justification::centred);
}

void MidiLearnComponent::resized() {}

void MidiLearnComponent::mouseDown(const juce::MouseEvent& event) {
    if (boxBounds.contains(event.getPosition())) {
        isLearning.store(true);
        repaint();
    }
}

void MidiLearnComponent::handleAsyncUpdate() {
    repaint();
}

juce::String MidiLearnComponent::noteNumberToName(int noteNumber) {
    if (noteNumber < 0 || noteNumber > 127)
        return {};

    static const char* noteNames[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
    int octave = (noteNumber / 12) - 1;
    int noteIndex = noteNumber % 12;

    return juce::String(noteNames[noteIndex]) + juce::String(octave);
}