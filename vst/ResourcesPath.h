#pragma once

#include <string>

// Returns the path to the VST plugin's Resources directory.
// On macOS: /path/to/Plugin.vst3/Contents/Resources
// Returns empty string if the path cannot be determined.
std::string getResourcesBasePath();
