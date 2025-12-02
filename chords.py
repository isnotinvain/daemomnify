import json
from abc import ABC, abstractmethod
from dataclasses import dataclass
from enum import Enum


def _load_chord_offsets(path):
    with open(path) as f:
        data = json.load(f)
        result = {}
        for name, chord_map in data.items():
            result[name] = {}
            for note_str, offsets in chord_map.items():
                result[name][int(note_str)] = offsets
        return result


@dataclass(frozen=True)
class ChordQualityData:
    nice_name: str
    json_file_key: str


class ChordQuality(Enum):
    MAJOR = ChordQualityData("Major", "MAJOR")
    MINOR = ChordQualityData("Minor", "MINOR")
    DOM_7 = ChordQualityData("Dominant 7th", "DOMINANT_7")


ChordQuality.all = list(ChordQuality)
ChordQuality.chord_region_size = 128 / len(ChordQuality.all)


class ChordConstructor(ABC):
    """
    Interface for constructing a chord given a quality and root note.
    Some constructors will return notes below the root, that's fine, think of
    the root as the "anchor" of the chord. Constructors can choose to use fixed octaves
    or be relative to the root.
    """

    @abstractmethod
    def construct_chord(self, quality: ChordQuality, root: int) -> list[int]: ...


class RootPositionConstructor(ChordConstructor):
    """
    Makes plain root position chords
    """

    OFFSETS = {
        ChordQuality.MAJOR: [0, 4, 7],
        ChordQuality.MINOR: [0, 3, 7],
        ChordQuality.DOM_7: [0, 3, 7, 10],
    }

    def __init__(self, add_sub_ref: list[bool]):
        self.add_sub = add_sub_ref

    def construct_chord(self, quality: ChordQuality, root: int) -> list[int]:
        offsets = self.OFFSETS[quality]
        n = [root + x for x in offsets]
        if self.add_sub is not None and self.add_sub[0]:
            n.append(root - 12)
        return n


class FileConstructor(ChordConstructor):
    """
    Loads chord offset voicings from a json file
    """

    def __init__(self, path):
        # dict[json_file_key, dict[note_class, list[offsets]]]
        self.data = _load_chord_offsets(path)

    def construct_chord(self, quality: ChordQuality, root: int) -> list[int]:
        lookup = self.data[quality.value.json_file_key]
        note_class = root % 12
        return lookup[note_class]


class PlainAscendingStrumConstructor(ChordConstructor):
    """
    Makes plain ascending strum sequences
    """

    TRIAD_OFFSETS = {
        ChordQuality.MAJOR: [0, 4, 7],
        ChordQuality.MINOR: [0, 3, 7],
        ChordQuality.DOM_7: [0, 3, 10],  # drop the 5th
    }

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
        triad = self.TRIAD_OFFSETS[quality]
        strum_offsets = self.make_strum_from_triad(triad)
        return [root + x for x in strum_offsets]


class OmnichordStrumConstructor(ChordConstructor):
    """
    Mimics the omnichord strum voicing algorithm
    """

    TRIAD_OFFSETS = {
        ChordQuality.MAJOR: [0, 4, 7],
        ChordQuality.MINOR: [0, 3, 7],
        ChordQuality.DOM_7: [0, 3, 10],  # drop the 5th
    }

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
        triad = self.TRIAD_OFFSETS[quality]
        res = []
        root_octave_start = self.find_lowest_f_sharp(root)
        for o in (-12, 0, 12, 24, 36):
            this_octave_start = root_octave_start + o
            for offset in triad:
                note = root + offset
                res.append(self.clamp(this_octave_start, note))

        return res[0:13]  # we only need 13 notes not 15
