# Omnify

Transforms any MIDI instrument into an omnichord / autoharp style instrument. Users play MIDI instruments like automatic harps - strumming on a mod wheel/plate while using pads to select chord qualities and keyboard notes to set chord roots. Outputs chords and strums on separate MIDI channels.

## Directory Structure
```
src/
├── PluginProcessor.cpp/h  # VST lifecycle, settings, APVTS params
├── PluginEditor.cpp/h     # VST UI layout
├── Omnify.cpp/h           # Core chord/strum processing engine
├── Daemomnify.cpp/h       # Background MIDI thread for direct hardware
├── MidiMessageScheduler.cpp/h  # Delayed MIDI delivery (strum timing)
├── datamodel/             # Settings, ChordQuality, MidiButton, VoicingStyle
├── ui/
│   ├── components/        # MidiLearnComponent, VariantSelector, etc.
│   ├── panels/            # ChordSettingsPanel, StrumSettingsPanel, ChordQualityPanel
│   └── LcarsLookAndFeel.h # Custom JUCE theme (LCARS colors, Orbitron font)
├── voicing_styles/        # Voicing algorithms (OmnichordChords, RootPosition, FromFile, etc.)
├── Resources/             # Fonts, default_settings.json
└── JUCE/                  # Git submodule (customized fork)
```

## Core Architecture
- **Omnify** (`Omnify.cpp`): State machine processing MIDI input. Tracks current chord, latch mode, strum timing. Delegates to voicing styles for note generation.
- **Daemomnify** (`Daemomnify.cpp`): juce::Thread that polls external MIDI hardware and feeds messages to Omnify engine.
- **OmnifySettings** (`datamodel/OmnifySettings.h`): Main config struct with MIDI device, channels, voicing styles, button mappings. Serializes to/from JSON.
- **VoicingStyle** (`datamodel/VoicingStyle.h`): Abstract template for chord generation. Subclasses implement `constructChord(quality, root) -> vector<int>`. Two registries: ChordVoicingRegistry and StrumVoicingRegistry.

## Data Model Key Classes
- `ChordQuality` - enum of 9 qualities: MAJOR, MINOR, DOM_7, MAJOR_7, MINOR_7, DIM_7, AUGMENTED, SUS_4, ADD_9
- `MidiButton` - Configurable trigger (note or CC, with button action: FLIP/ON/OFF)
- `ChordQualitySelectionStyle` - Variant: ButtonPerChordQuality or CCRangePerChordQuality
- `RealtimeParams` - Thread-safe atomics for strumGateTimeMs, strumCooldownMs

## Voicing Styles
Located in `src/voicing_styles/`:
- **OmnichordChords/OmnichordStrum** - Authentic Omnichord voicings with relative F# octave positioning
- **RootPosition** - Simple root position triads
- **PlainAscending** - Ascending arpeggio
- **FromFile** - Load voicing from JSON file (supports offset-based or absolute notes)

## C++ Dependencies
- JUCE is a git submodule in src/ - we are free to add bugfixes and features to JUCE as needed
- nlohmann/json (header-only) in src/nlohmann/

## Threading
- APVTS for audio thread-safe parameters
- `std::atomic<shared_ptr<OmnifySettings>>` for settings
- Daemomnify runs on dedicated background thread
- MidiLearnComponent uses atomics for thread-safe learning state

## Patterns
- Complex nested settings stored as JSON blob in VST (simpler than exposing every field)
- LCARS (Star Trek) visual theme in the VST UI
- Registry pattern for voicing styles (registered at startup, referenced by type name)
- JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR on all classes

## Commands
- You have available to you a helper script for running commands that output a lot.
  - it is: .claude/hooks/run-with-log.sh <log_file> <command>
  - It will `tee` the <command>'s stdout to <log_file> and back to you
  - You can use | head or | tail on it
  - If you don't get enough from that, don't re-run <command> -- inspect the <log_file>
  - see "Compile VST" below for an example
- Compile VST: `.claude/hooks/run-with-log.sh /tmp/vst-compile.log ./src/compile.sh -d` (drop `-d` for release)

## Interaction style
- Don't make Edits immediately after propsing an alternate solution -- ask for confirmation first.
- If you explain multiple ways something could be done, ask me which one I want before proceding with Edits
- When writing brand new files, pause after each one so we can discuss before moving on (unless of course you are in allow edits mode, then do your thing)

## Coding Style
- Don't add comments that are pretty obvious from the code they are commenting, or from the function signature and name. If the code is clear, if the variable names are clear, and if the function signatures are clear, skip the comments entirely. Do add comments for gotchas / bug fixes / workarounds, methods with important rules / constraints that aren't immediately obvious, or complicated algorithms.
