#pragma once

#include <json.hpp>

enum class ButtonAction { FLIP, ON, OFF };

// Unified MIDI button - can be triggered by note or CC
class MidiButton {
   public:
    int note = -1;
    int cc = -1;
    bool ccIsToggle = false;

    MidiButton() = default;
    MidiButton(int note, int cc, bool isToggle);

    // Factory methods for common configurations
    static MidiButton fromNote(int noteNum);
    static MidiButton fromCC(int ccNum, bool toggle = false);

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(MidiButton, note, cc, ccIsToggle)
};