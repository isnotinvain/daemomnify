# Daemomnify

Transforms any MIDI instrument into an omnichord (autoharp-style instrument). Two major components:
1) A python script that handles midi in + transformations + midi out
2) A C++ VST for UI only

## Architecture
- Python core in `src/daemomnify/` - Pydantic settings, MIDI processing, chord voicing logic
- VST plugin in `vst/` - C++17 with JUCE + Foleys GUI Magic
- Code generation bridges Python↔C++: `vst/generate_vst_params.py` introspects Pydantic models to generate `GeneratedParams.h/cpp` and `GeneratedAdditionalSettings.h/cpp` (don't edit these directly)

## C++ Dependencies
- Both JUCE and Foleys GUI Magic are git submodules in vst/
- We are free to add bugfixes and features to JUCE or Foleys as needed
- You can explore these codebases for debugging as well

## Key Files
- `src/daemomnify/settings.py` - All configuration (Pydantic models with VST param annotations)
- `src/daemomnify/omnify.py` - Core state machine (chord generation, strum logic)
- `vst/PluginProcessor.cpp` - Main VST plugin logic
- `vst/ui/GuiItems.h` - Custom Foleys GUI item factories
- `vst/Resources/magic.xml` - GUI layout (Foleys GUI Magic)

## Patterns
- Complex nested settings stored as JSON blob in VST (simpler than exposing every field)
- LCARS (Star Trek) visual theme in the VST UI

## Commands
- Run daemon: `uv run python -m daemomnify`
- Run tests: `uv run pytest`
- Regenerate VST params after changing settings.py: `uv run python vst/generate_vst_params.py`
- Python: use `uv run`
- Install python packages: `/install`
- Compile VST: `./vst/compile.sh -d` (drop `-d` for release)
- For long commands or commands that output a lot of tokens (e.g. compiling):
  - Use: `.claude/hooks/run-with-log.sh <command>`
  - Example: `.claude/hooks/run-with-log.sh ./vst/compile.sh -d`
  - This `tee`s the output to a file and to stdout. You can head/tail the stdout.
  - If you need more than what you got from head/tail, you can inspect the log file instead or re-running.
- Don't make Edits immediately after propsing an alternate solution -- ask for confirmation first.
- If you explain multiple ways something could be done, ask me which one I want before proceding with Edits