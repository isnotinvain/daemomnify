#include "MidiButton.h"

MidiButton::MidiButton(int noteIn, int ccIn, bool isToggleIn) : note(noteIn), cc(ccIn), ccIsToggle(isToggleIn) {}

MidiButton MidiButton::fromNote(int noteNum) { return MidiButton{noteNum, -1, false}; }

MidiButton MidiButton::fromCC(int ccNum, bool toggle) { return MidiButton{-1, ccNum, toggle}; }
