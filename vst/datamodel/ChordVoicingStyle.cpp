#include "ChordVoicingStyle.h"

#include <stdexcept>

void ChordVoicingStyleRegistry::registerStyle(const std::string& typeName,
                                              std::shared_ptr<ChordVoicingStyle> style,
                                              ChordVoicingStyleFactory factory) {
    registry[typeName] = ChordVoicingStyleEntry{std::move(style), std::move(factory)};
}

std::shared_ptr<ChordVoicingStyle> ChordVoicingStyleRegistry::from_json(
    const nlohmann::json& j) const {
    if (!j.contains("type")) {
        throw std::runtime_error("ChordVoicingStyle JSON missing 'type' field");
    }

    std::string typeName = j.at("type").get<std::string>();
    auto it = registry.find(typeName);
    if (it == registry.end()) {
        throw std::runtime_error("Unknown ChordVoicingStyle type: " + typeName);
    }

    return it->second.from_json(j);
}
