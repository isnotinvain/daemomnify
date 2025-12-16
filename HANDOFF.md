# Handoff Plan: Complete MidiThread Integration

## Completed

1. **MidiThread** - Full implementation (h/cpp), compiles clean
2. **Removed DaemonManager** - Python daemon no longer needed
3. **Basic integration** - PluginProcessor creates and starts MidiThread pipeline
4. **Realtime params** - strum gate/cooldown update RealtimeParams atomics directly

## Next: Replace GeneratedSettings with OmnifySettings

The plugin currently has two settings types:
- `GeneratedSettings::DaemomnifySettings` - used by UI/serialization (std::variant based)
- `OmnifySettings` - used by Omnify (shared_ptr<VoicingStyle> based)

**Goal:** Delete GeneratedSettings, use OmnifySettings everywhere.

### Key differences to bridge

| GeneratedSettings | OmnifySettings |
|-------------------|----------------|
| `std::variant<RootPositionStyle, ...>` | `shared_ptr<VoicingStyle<Chord>>` |
| `std::variant<MidiNoteButton, MidiCCButton>` | `MidiButton` class |
| `strum_gate_time_secs` (double) | `strumGateTimeMs` (int) |
| `midi_device_name` | `midiDeviceName` |

### Steps

1. Update `PluginProcessor.h` to use `OmnifySettings` instead of `GeneratedSettings::DaemomnifySettings`
2. Update `valueChanged()` to build OmnifySettings fields directly
3. Update `loadSettingsFromValueTree()` / `saveSettingsToValueTree()` to use `OmnifySettings::to_json/from_json`
4. Update `loadDefaultSettings()` to parse JSON into OmnifySettings
5. Wire up `omnify->updateSettings()` when settings change
6. Wire up `midiThread->setInputDevice()` when device changes
7. Delete `GeneratedSettings.h`, `GeneratedSettings.cpp`, and `vst/generate_settings.py`
8. Update UI components that reference GeneratedSettings types

### Reference files

- `vst/PluginProcessor.cpp` - main integration point
- `vst/datamodel/OmnifySettings.h` - target settings type
- `vst/datamodel/VoicingStyle.h` - polymorphic voicing styles
- `vst/voicing_styles/*.h` - concrete implementations (RootPosition, FromFile, Omni84, etc.)
- `vst/datamodel/MidiButton.h` - button handling
- `vst/datamodel/ChordQualitySelectionStyle.h` - chord quality selector
