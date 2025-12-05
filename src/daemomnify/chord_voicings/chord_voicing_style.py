from pydantic import BaseModel

from daemomnify.chord_quality import ChordQuality


class ChordVoicingStyle(BaseModel):
    """
    Interface for constructing a chord given a quality and root note.
    Some styles will return notes below the root, that's fine, think of
    the root as the "anchor" of the chord. Styles can choose to use fixed octaves
    or be relative to the root.
    """

    def construct_chord(self, quality: ChordQuality, root: int) -> list[int]:
        raise NotImplementedError
