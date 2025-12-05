"""
VST Parameter annotations for automatic C++ code generation.

Use these with pydantic Field() to control how fields are exposed as VST parameters.
"""

from dataclasses import dataclass
from typing import Any


@dataclass
class VSTParam:
    """Base class for VST parameter metadata."""
    pass


@dataclass
class VSTInt(VSTParam):
    """Integer parameter (slider or number box)."""
    min: int = 0
    max: int = 127
    default: int = 0
    label: str | None = None


@dataclass
class VSTFloat(VSTParam):
    """Float parameter (slider)."""
    min: float = 0.0
    max: float = 1.0
    default: float = 0.0
    label: str | None = None


@dataclass
class VSTBool(VSTParam):
    """Boolean parameter (toggle)."""
    default: bool = False
    label: str | None = None


@dataclass
class VSTChoice(VSTParam):
    """Choice parameter (combo box). Choices are auto-derived from union types."""
    default_index: int = 0
    label: str | None = None


@dataclass
class VSTIntChoice(VSTParam):
    """Integer range as a choice parameter (combo box). For small ranges like MIDI channel 1-16."""
    min: int = 0
    max: int = 127
    default: int = 0
    label: str | None = None


@dataclass
class VSTString(VSTParam):
    """String parameter (text field)."""
    default: str = ""
    label: str | None = None


@dataclass
class VSTSkip(VSTParam):
    """Skip this field - don't generate a VST parameter for it."""
    pass


@dataclass
class VSTChordQualityMap(VSTParam):
    """
    Special handler for dict[int, ChordQuality] mappings.
    Expands to one int parameter per chord quality (major, minor, dom7).
    """
    min: int = 0
    max: int = 127
    label_prefix: str | None = None


def get_vst_param(field_info) -> VSTParam | None:
    """Extract VSTParam metadata from a pydantic FieldInfo if present."""
    if field_info.json_schema_extra and isinstance(field_info.json_schema_extra, dict):
        return field_info.json_schema_extra.get("vst")
    return None