#include "MidiButton.h"

MidiButton::MidiButton(int note, int cc, bool isToggle) : note(note), cc(cc), ccIsToggle(isToggle) {}

MidiButton MidiButton::fromNote(int noteNum) { return MidiButton{noteNum, -1, false}; }

MidiButton MidiButton::fromCC(int ccNum, bool toggle) { return MidiButton{-1, ccNum, toggle}; }
