from enum import Enum
from pathlib import Path
from typing import Annotated, Literal

from pydantic import BaseModel, Field

from daemomnify import chords
from daemomnify.chords import (
    FileStyle,
    Omni84Style,
    OmnichordStrumStyle,
    PlainAscendingStrumStyle,
    RootPositionStyle,
)


class NotePerChordQuality(BaseModel):
    type: Literal["NotePerChordQuality"] = "NotePerChordQuality"
    note_mapping: dict[int, chords.ChordQuality]

    def handles_message(self, msg):
        if msg.type == "note_on" and msg.note in self.note_mapping:
            return self.note_mapping[msg.note]
        return None


class CCPerChordQuality(BaseModel):
    type: Literal["CCPerChordQuality"] = "CCPerChordQuality"
    cc_mapping: dict[int, chords.ChordQuality]

    def handles_message(self, msg):
        if msg.type == "control_change" and msg.control in self.cc_mapping:
            return self.cc_mapping[msg.control]
        return None


class CCRangePerChordQuality(BaseModel):
    type: Literal["CCRangePerChordQuality"] = "CCRangePerChordQuality"
    cc: int

    def handles_message(self, msg):
        if msg.type == "control_change" and msg.control == self.cc:
            chord_index = min(int(msg.value / chords.ChordQuality.chord_region_size), len(chords.ChordQuality.all) - 1)
            return chords.ChordQuality.all[chord_index]
        return None


ChordQualitySelectionStyle = Annotated[NotePerChordQuality | CCPerChordQuality | CCRangePerChordQuality, Field(discriminator="type")]


class ButtonAction(Enum):
    FLIP = 0
    ON = 1
    OFF = 2


class MidiNoteButton(BaseModel):
    type: Literal["MidiNoteButton"] = "MidiNoteButton"
    note: int

    def handles_message(self, msg):
        if msg.type == "note_on" and msg.note == self.note:
            return ButtonAction.FLIP
        return None


class MidiCCButton(BaseModel):
    type: Literal["MidiCCButton"] = "MidiCCButton"
    cc: int
    is_toggle: bool

    def handles_message(self, msg):
        if msg.type == "control_change" and msg.control == self.cc:
            if self.is_toggle:
                return ButtonAction.ON if msg.value > 64 else ButtonAction.OFF
            else:
                return ButtonAction.FLIP
        return None


MidiButton = Annotated[MidiNoteButton | MidiCCButton, Field(discriminator="type")]


ChordStyleConfig = Annotated[RootPositionStyle | FileStyle | Omni84Style, Field(discriminator="type")]
StrumStyleConfig = Annotated[PlainAscendingStrumStyle | OmnichordStrumStyle, Field(discriminator="type")]


class DaemomnifySettings(BaseModel):
    midi_device_name: str
    chord_voicing_style: ChordStyleConfig
    chord_channel: int
    strum_channel: int
    strum_voicing_style: StrumStyleConfig
    strum_cooldown_secs: float
    strum_gate_time_secs: float
    chord_quality_selection_style: ChordQualitySelectionStyle
    strum_plate_cc: int
    latch_toggle_button: MidiButton
    stop_button: MidiButton

    # TODO: just make a set of these in the constructor
    def is_note_control_note(self, note: int) -> bool:
        match self.chord_quality_selection_style:
            case NotePerChordQuality() as m:
                if note in m.note_mapping:
                    return True
            case _:
                pass
        match self.latch_toggle_button:
            case MidiNoteButton() as b:
                if b.note == note:
                    return True
            case _:
                pass
        match self.stop_button:
            case MidiNoteButton() as b:
                if b.note == note:
                    return True
            case _:
                pass
        return False


def load_settings(path: Path = Path("daemomnify_settings.json")) -> DaemomnifySettings | None:
    try:
        json_str = path.read_text()
        return DaemomnifySettings.model_validate_json(json_str)
    except FileNotFoundError:
        print(f"I don't see a settings file at: {path}...")
        return None


def save_settings(settings: DaemomnifySettings, path: Path = Path("daemomnify_settings.json")) -> None:
    path.write_text(settings.model_dump_json(indent=2))


DEFAULT_SETTINGS: DaemomnifySettings = DaemomnifySettings(
    midi_device_name="Launchkey Mini MK3 MIDI Port",
    chord_voicing_style=RootPositionStyle(),
    chord_channel=1,
    strum_channel=2,
    strum_voicing_style=PlainAscendingStrumStyle(),
    strum_cooldown_secs=0.3,  # TODO: use cc
    strum_gate_time_secs=0.5,  # TODO: use cc
    chord_quality_selection_style=NotePerChordQuality(
        note_mapping={
            0: chords.ChordQuality.MAJOR,
            1: chords.ChordQuality.MINOR,
            2: chords.ChordQuality.DOM_7,
        }
    ),
    strum_plate_cc=1,
    latch_toggle_button=MidiCCButton(cc=102, is_toggle=True),
    stop_button=MidiCCButton(cc=103, is_toggle=False),
)

if __name__ == "__main__":
    print("Writing default settings file...")
    save_settings(DEFAULT_SETTINGS)
    print("Done!")
    print("Reading...")
    print(load_settings())
