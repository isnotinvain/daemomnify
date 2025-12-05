from typing import Literal

from daemomnify.chords import ChordQuality, ChordVoicingStyle


class RootPositionStyle(ChordVoicingStyle):
    """
    Makes plain root position chords
    """

    type: Literal["RootPositionChordStyle"] = "RootPositionChordStyle"

    def construct_chord(self, quality: ChordQuality, root: int) -> list[int]:
        offsets = quality.value.offsets
        return [root + x for x in offsets]
