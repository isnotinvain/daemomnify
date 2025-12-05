from dataclasses import dataclass
from enum import Enum

from pydantic import BaseModel
from pydantic_core import core_schema


@dataclass(frozen=True)
class ChordQualityData:
    nice_name: str
    suffix: str
    offsets: tuple[int, ...]
    triad_offsets: tuple[int, ...]


class ChordQuality(Enum):
    MAJOR = ChordQualityData("Major", "maj", (0, 4, 7), triad_offsets=(0, 4, 7))
    MINOR = ChordQualityData("Minor", "m", (0, 3, 7), triad_offsets=(0, 3, 7))
    DOM_7 = ChordQualityData("Dominant 7th", "7", (0, 4, 7, 10), triad_offsets=(0, 4, 10))
    MAJOR_7 = ChordQualityData("Major 7th", "maj7", (0, 4, 7, 11), triad_offsets=(0, 4, 11))
    MINOR_7 = ChordQualityData("Minor 7th", "m7", (0, 3, 7, 10), triad_offsets=(0, 3, 10))
    DIM_7 = ChordQualityData("Diminished 7th", "dim7", (0, 3, 6, 9), triad_offsets=(0, 3, 9))
    AUGMENTED = ChordQualityData("Augmented", "aug", (0, 4, 8), triad_offsets=(0, 4, 8))
    SUS_4 = ChordQualityData("Suspended 4th", "sus4", (0, 5, 7), triad_offsets=(0, 5, 7))
    ADD_9 = ChordQualityData("Add 9", "add9", (0, 4, 7, 14), triad_offsets=(0, 7, 14))

    @classmethod
    def __get_pydantic_core_schema__(cls, source_type, handler):
        return core_schema.no_info_plain_validator_function(
            lambda v: v if isinstance(v, cls) else cls[v],
            serialization=core_schema.plain_serializer_function_ser_schema(lambda v: v.name),
        )


ChordQuality.all = list(ChordQuality)
ChordQuality.chord_region_size = 128 / len(ChordQuality.all)


class ChordVoicingStyle(BaseModel):
    """
    Interface for constructing a chord given a quality and root note.
    Some styles will return notes below the root, that's fine, think of
    the root as the "anchor" of the chord. Styles can choose to use fixed octaves
    or be relative to the root.
    """

    def construct_chord(self, quality: ChordQuality, root: int) -> list[int]:
        raise NotImplementedError
