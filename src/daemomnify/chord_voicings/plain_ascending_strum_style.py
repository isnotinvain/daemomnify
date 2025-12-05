from typing import Literal

from daemomnify.chord_quality import ChordQuality
from daemomnify.chord_voicings.chord_voicing_style import ChordVoicingStyle


class PlainAscendingStrumStyle(ChordVoicingStyle):
    """
    Makes plain ascending strum sequences
    """

    type: Literal["PlainAscendingStrumStyle"] = "PlainAscendingStrumStyle"

    @staticmethod
    def make_strum_from_triad(offsets):
        res = []
        for shift in (-12, 0, 12, 24):
            for o in offsets:
                res.append(shift + o)
        # 13th element is one more octave up
        res.append(36)
        return res

    def construct_chord(self, quality: ChordQuality, root: int) -> list[int]:
        triad = quality.value.triad_offsets
        strum_offsets = self.make_strum_from_triad(triad)
        return [root + x for x in strum_offsets]
