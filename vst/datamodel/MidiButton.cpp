#include "MidiButton.h"

MidiButton::MidiButton(int note, int cc, bool ccIsToggle) : note(note), cc(cc), ccIsToggle(ccIsToggle) {}

MidiButton MidiButton::fromNote(int noteNum) { return MidiButton{noteNum, -1, false}; }

MidiButton MidiButton::fromCC(int ccNum, bool toggle) { return MidiButton{-1, ccNum, toggle}; }

std::optional<ButtonAction> MidiButton::handle(const juce::MidiMessage& msg) const {
    // velocity == 0 means note off in some devices
    if (note >= 0 && msg.isNoteOn() && msg.getVelocity() != 0 && msg.getNoteNumber() == note) {
        return ButtonAction::FLIP;
    }

    if (cc >= 0 && msg.isController() && msg.getControllerNumber() == cc) {
        if (ccIsToggle) {
            if (msg.getControllerValue() > 63) {
                return ButtonAction::FLIP;
            }
        } else {
            return msg.getControllerValue() > 63 ? ButtonAction::ON : ButtonAction::OFF;
        }
    }

    return std::nullopt;
}
