#pragma once

#include <functional>
#include <json.hpp>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "ChordQuality.h"

class ChordVoicingStyle;

using ChordVoicingStyleFactory =
    std::function<std::shared_ptr<ChordVoicingStyle>(const nlohmann::json&)>;

struct ChordVoicingStyleEntry {
    std::shared_ptr<ChordVoicingStyle> instance;
    ChordVoicingStyleFactory from_json;
};

// Abstract base class for chord voicing styles.
class ChordVoicingStyle {
   public:
    virtual ~ChordVoicingStyle() = default;
    virtual std::string displayName() const = 0;
    virtual std::vector<int> constructChord(ChordQuality quality, int root) const = 0;
    virtual void to_json(nlohmann::json& j) const = 0;
};

// Registry of available chord voicing styles - one per plugin instance
class ChordVoicingStyleRegistry {
   public:
    ChordVoicingStyleRegistry() = default;

    void registerStyle(const std::string& typeName, std::shared_ptr<ChordVoicingStyle> style,
                       ChordVoicingStyleFactory factory);
    std::shared_ptr<ChordVoicingStyle> from_json(const nlohmann::json& j) const;
    std::map<std::string, ChordVoicingStyleEntry>& getRegistry() { return registry; }

   private:
    std::map<std::string, ChordVoicingStyleEntry> registry;
};