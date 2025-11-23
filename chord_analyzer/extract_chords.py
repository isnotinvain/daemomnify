from pathlib import Path

from basic_pitch import ICASSP_2022_MODEL_PATH
from basic_pitch.inference import predict


def midi_to_note_name(midi_number: int) -> str:
    note_names = ["C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"]
    octave = (midi_number // 12) - 1
    note = note_names[midi_number % 12]
    return f"{note}{octave}"


def extract_chord_voicing(wav_path: Path) -> dict:
    """Extract MIDI notes from a WAV file using Basic Pitch."""
    model_output, midi_data, note_events = predict(
        str(wav_path),
        ICASSP_2022_MODEL_PATH,
        onset_threshold=0.5,
        frame_threshold=0.3,
        minimum_note_length=127,
    )

    midi_notes = set()
    for instrument in midi_data.instruments:
        for note in instrument.notes:
            midi_notes.add(note.pitch)

    midi_notes_sorted = sorted(list(midi_notes))
    note_names = [midi_to_note_name(n) for n in midi_notes_sorted]
    voicing = ", ".join(note_names) if note_names else "No notes detected"

    return {
        "midi_notes": midi_notes_sorted,
        "note_names": note_names,
        "voicing": voicing,
        "num_notes": len(midi_notes_sorted),
    }


def analyze_chord_folder(folder_path: Path) -> dict[str, dict]:
    """Analyze all WAV files in a folder."""
    results = {}
    for wav_file in sorted(folder_path.glob("*.wav")):
        print(f"Analyzing: {wav_file.name}")
        try:
            result = extract_chord_voicing(wav_file)
            results[wav_file.name] = result
            print(f"  -> {result['voicing']} ({result['num_notes']} notes)")
        except Exception as e:
            print(f"  -> Error: {e}")
            results[wav_file.name] = {"error": str(e)}
    return results


def note_name_to_class(note_name: str) -> int:
    """Convert note name (e.g., 'C', 'C#', 'A#') to MIDI note class (0-11)."""
    note_map = {"C": 0, "C#": 1, "D": 2, "D#": 3, "E": 4, "F": 5, "F#": 6, "G": 7, "G#": 8, "A": 9, "A#": 10, "B": 11}
    return note_map.get(note_name.upper(), -1)


def chord_type_to_quality(chord_type: int) -> str:
    """Convert chord type number to chord quality name."""
    chord_quality_map = {1: "Major", 2: "Minor", 3: "Dominant 7th", 4: "Minor 7th", 5: "Major 7th", 6: "Minor 9th", 7: "Augmented"}
    return chord_quality_map.get(chord_type, "Unknown")


def extract_note_from_filename(filename: str) -> tuple[int, str, int]:
    """Extract chord type, note class, and note name from filename like 'Chords_1A.wav' or 'Chords_3C#.wav'."""
    # Remove extension and split by underscore
    name = filename.replace(".wav", "")
    parts = name.split("_")

    if len(parts) >= 2:
        # Extract chord type (first digit) and note part (everything after)
        chord_type = int(parts[1][0])  # First digit is chord type
        note_part = parts[1][1:]  # Rest is the note name
        note_class = note_name_to_class(note_part)
        return chord_type, note_class, note_part

    return -1, -1, ""


if __name__ == "__main__":
    import json
    import sys

    chord_dir = Path.home() / "foo/bar/baz"
    output_file = Path("chord_mappings.json")

    if not chord_dir.exists():
        print(f"Error: Directory does not exist: {chord_dir}")
        sys.exit(1)

    print(f"Analyzing chord files in: {chord_dir}\n")
    results = analyze_chord_folder(chord_dir)

    # Create mapping from (chord_type, note_class) to chord information
    chord_mappings = {}

    for filename, result in results.items():
        if "error" in result:
            print(f"{filename}: ERROR - {result['error']}")
            continue

        # Extract chord type and note class from filename
        chord_type, note_class, note_name = extract_note_from_filename(filename)

        if note_class == -1 or chord_type == -1:
            print(f"{filename}: Could not extract note/chord type from filename")
            continue

        if result["midi_notes"]:
            # Use a unique key combining chord type and note class
            key = f"{chord_type}_{note_class}"
            chord_quality = chord_type_to_quality(chord_type)

            chord_mappings[key] = {
                "chord_type": chord_type,
                "chord_quality": chord_quality,
                "note_class": note_class,
                "note_name": note_name,
                "midi_notes": [int(n) for n in result["midi_notes"]],
                "note_names": result["note_names"],
                "filename": filename,
            }
            print(f"{filename}: {result['voicing']} -> {chord_quality}, Note class: {note_class} ({note_name})")
        else:
            print(f"{filename}: No notes detected")

    # Write JSON output
    with open(output_file, "w") as f:
        json.dump(chord_mappings, f, indent=2)

    print(f"\n✓ Chord mappings written to: {output_file}")
    print(f"  Total mappings: {len(chord_mappings)}")
