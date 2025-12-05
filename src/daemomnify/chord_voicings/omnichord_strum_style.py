from typing import Literal

from daemomnify.chord_quality import ChordQuality
from daemomnify.chord_voicings.chord_voicing_style import ChordVoicingStyle


class OmnichordStrumStyle(ChordVoicingStyle):
    """
    Mimics the omnichord strum voicing algorithm
    """

    type: Literal["OmnichordStrumStyle"] = "OmnichordStrumStyle"

    @classmethod
    def vst_label(cls) -> str:
        return "Omnichord"

    @staticmethod
    def clamp(lowest_f_sharp: int, n: int) -> int:
        pitch_class = n % 12
        distance_from_f_sharp = pitch_class - 6
        if distance_from_f_sharp < 0:
            distance_from_f_sharp += 12
        return lowest_f_sharp + distance_from_f_sharp

    @staticmethod
    def find_lowest_f_sharp(root: int) -> int:
        root_shifted = max(0, root - 6)
        root_octave = root_shifted // 12
        lowest_f_sharp = (root_octave * 12) + 6
        return lowest_f_sharp

    def construct_chord(self, quality: ChordQuality, root: int) -> list[int]:
        triad = quality.value.triad_offsets
        res = []
        root_octave_start = self.find_lowest_f_sharp(root)
        for o in (-12, 0, 12, 24, 36):
            this_octave_start = root_octave_start + o
            for offset in triad:
                note = root + offset
                res.append(self.clamp(this_octave_start, note))

        return res[0:13]  # we only need 13 notes not 15
