#pragma once

#include <json.hpp>
#include <map>
#include <variant>

#include "ChordQuality.h"

class ButtonPerChordQuality {
   public:
    std::map<int, ChordQuality> notes;
    std::map<int, ChordQuality> ccs;

    ButtonPerChordQuality() = default;
    ButtonPerChordQuality(std::map<int, ChordQuality> notes, std::map<int, ChordQuality> ccs);

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(ButtonPerChordQuality, notes, ccs)
};

class CCRangePerChordQuality {
   public:
    int cc = 0;

    CCRangePerChordQuality() = default;
    CCRangePerChordQuality(int cc);

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(CCRangePerChordQuality, cc)
};

class ChordQualitySelectionStyle {
   public:
    std::variant<ButtonPerChordQuality, CCRangePerChordQuality> value;

    ChordQualitySelectionStyle() = default;
    ChordQualitySelectionStyle(ButtonPerChordQuality v);
    ChordQualitySelectionStyle(CCRangePerChordQuality v);

    nlohmann::json to_json() const;
    static ChordQualitySelectionStyle from_json(const nlohmann::json& j);
};