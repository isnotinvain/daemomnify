#pragma once

#include <juce_audio_basics/juce_audio_basics.h>

#include <json.hpp>
#include <optional>

enum class ButtonAction { FLIP, ON, OFF };

// Unified MIDI button - can be triggered by note or CC
class MidiButton {
   public:
    int note = -1;
    int cc = -1;
    bool ccIsToggle = false;

    MidiButton() = default;
    MidiButton(int note, int cc, bool ccIsToggle);

    static MidiButton fromNote(int noteNum);
    static MidiButton fromCC(int ccNum, bool toggle = false);

    std::optional<ButtonAction> handle(const juce::MidiMessage& msg) const;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(MidiButton, note, cc, ccIsToggle)
};