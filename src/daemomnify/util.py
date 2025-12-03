# just range, with inclusive 2nd arg
def irange(x, y):
    return range(x, y + 1)


def clamp_note(note):
    if note > 127:
        return (note % 12) + 108  # Clamp to highest octave
    if note < 0:
        return abs(note) % 12
    return note


PITCH_CLASSES = [
    "C",
    "C#/Db",
    "D",
    "D#/Eb",
    "E",
    "F",
    "F#/Gb",
    "G",
    "G#/Ab",
    "A",
    "A#/Bb",
    "B",
]

# MIDI 36 = C1 (Ableton convention), so octave = (midi - 36) // 12 + 1
# which simplifies to (midi - 24) // 12
NOTE_NAMES = {midi: f"{PITCH_CLASSES[midi % 12]}{(midi - 24) // 12}" for midi in range(128)}
