from typing import ClassVar, Literal

from daemomnify.chord_quality import ChordQuality
from daemomnify.chord_voicings.chord_voicing_style import ChordVoicingStyle


class Omni84Style(ChordVoicingStyle):
    """
    Just outputs a single root note, in the octave corresponding to the chord quality using omni-84s octave layout
    Makes it easy to play omnify with omni-84 samples
    """

    type: Literal["Omni84Style"] = "Omni84Style"

    @classmethod
    def vst_label(cls) -> str:
        return "Omni-84"

    OCTAVE_BEGIN_MAP: ClassVar[dict[ChordQuality, int]] = {ChordQuality.MAJOR: 36, ChordQuality.MINOR: 48, ChordQuality.DOM_7: 60}

    def construct_chord(self, quality: ChordQuality, root: int) -> list[int]:
        pitch_class = root % 12
        octave_begin = self.OCTAVE_BEGIN_MAP[quality]
        return [octave_begin + pitch_class]
