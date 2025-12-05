from typing import Literal

from daemomnify.chord_quality import ChordQuality
from daemomnify.chord_voicings.chord_voicing_style import ChordVoicingStyle


class RootPositionStyle(ChordVoicingStyle):
    """
    Makes plain root position chords
    """

    type: Literal["RootPositionChordStyle"] = "RootPositionChordStyle"

    @classmethod
    def vst_label(cls) -> str:
        return "Root Position"

    def construct_chord(self, quality: ChordQuality, root: int) -> list[int]:
        offsets = quality.value.offsets
        return [root + x for x in offsets]
