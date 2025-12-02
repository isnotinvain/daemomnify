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


# _chord_data = _load_chord_offsets(path=Path(__file__).parent / "chord_offsets.json")


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


class ChordFormer(ABC):
    @abstractmethod
    def notes(self, root: int) -> list[int]: ...


class FixedPatternFormer(ChordFormer):
    """
    offsets are semitones from the root
    add_sub is a mutable container for whether the user has toggled a bass root note
    """

    def __init__(self, offsets: list[int], add_sub: list[bool]):
        self.offsets = offsets
        # list of single element, used as a mutable container (or None)
        self.add_sub = add_sub

    def notes(self, root: int) -> list[int]:
        n = [root + x for x in self.offsets]
        if self.add_sub is not None and self.add_sub[0]:
            n.append(root - 12)
        return n


class LookupPatternFormer(ChordFormer):
    """
    lookup is dict[note_class, offsets]
    offsets are semitones from the root
    """

    def __init__(self, lookup: dict[int, list[int]]):
        self.lookup = lookup

    def notes(self, root: int) -> list[int]:
        note_class = root % 12
        return self.lookup[note_class]


class ChordVoicer(ABC):
    @abstractmethod
    def formers(self) -> dict[ChordQuality, ChordFormer]: ...


class RootPositionChordVoicer(ChordVoicer):
    """
    Makes plain root position chords
    """

    def __init__(self, add_sub_ref: list[bool]):
        self.add_sub = add_sub_ref
        self._formers = {
            ChordQuality.MAJOR: FixedPatternFormer(offsets=[0, 4, 7], add_sub=self.add_sub),
            ChordQuality.MINOR: FixedPatternFormer(offsets=[0, 3, 7], add_sub=self.add_sub),
            ChordQuality.DOM_7: FixedPatternFormer(offsets=[0, 3, 7, 10], add_sub=self.add_sub),
        }

    def formers(self) -> dict[ChordQuality, ChordFormer]:
        return self._formers


class FileChordVoicer(ChordVoicer):
    """
    Loads chord offset voicings from a json file
    """

    def __init__(self, path):
        # dict[json_file_key, dict[note_class, list[offsets]]]
        self.data = _load_chord_offsets(path)
        self._formers = {}
        for quality in ChordQuality.all:
            self._formers[quality] = LookupPatternFormer(self.data[quality.value.json_file_key])

    def formers(self) -> dict[ChordQuality, ChordFormer]:
        return self._formers


class PlainAscendingStrumVoicer(ChordVoicer):
    """
    Makes plain ascending strum sequences
    """

    @staticmethod
    def make_strum_from_triad(pattern):
        res = []
        for shift in (-12, 0, 12, 24):
            for o in pattern:
                res.append(shift + o)

        # 13th element is one more octave up
        res.append(36)

    @staticmethod
    def make_strum_from_tetrad(pattern):
        """
        just drops the 5th
        """
        root, third, fifth, seventh = pattern
        return PlainAscendingStrumVoicer.make_strum_from_triad([root, third, seventh])

    @staticmethod
    def make_strum_from_chord_offsets(offsets):
        match len(offsets):
            case 3:
                return PlainAscendingStrumVoicer.make_strum_from_triad(offsets)
            case 4:
                return PlainAscendingStrumVoicer.make_strum_from_tetrad(offsets)
            case _:
                raise ValueError("offsets should be length 3 or 4")

    def __init__(self):
        self._formers = {}
        root_voicer = RootPositionChordVoicer(None)
        for quality, former in root_voicer.formers().items():
            strum_offsets = PlainAscendingStrumVoicer.make_strum_from_chord_offsets(former.offsets)
            self._formers[quality] = FixedPatternFormer(strum_offsets, None)

    def formers(self) -> dict[ChordQuality, ChordFormer]:
        return self._formers


class OmnichordStrumFormer(ChordFormer):
    def __init__(self, triad: list[int]):
        self.triad = triad

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

    def notes(self, root: int) -> list[int]:
        res = []
        root_octave_start = OmnichordStrumFormer.find_lowest_f_sharp(root)
        for o in (-12, 0, 12, 24, 36):
            this_octave_start = root_octave_start + o
            for n in self.triad:
                res.append(OmnichordStrumFormer.clamp(this_octave_start, n))

        return res[0:13]  # we only need 13 notes not 15


class OmnichordStrumVoicer(ChordVoicer):
    """
    Mimics the omnichord strum voicing algorithm
    """

    def __init__(self):
        self._formers = {}
        root_voicer = RootPositionChordVoicer(None)
        for quality, former in root_voicer.formers().items():
            match former.offsets:
                case 3:
                    triad = former.offsets
                case 4:
                    root, third, fifth, seventh = former.offsets
                    # drop 5th
                    triad = [root, third, seventh]
                case _:
                    raise ValueError("offsets should be length 3 or 4")

            self._formers[quality] = OmnichordStrumFormer(triad)

    def formers(self) -> dict[ChordQuality, ChordFormer]:
        return self._formers
