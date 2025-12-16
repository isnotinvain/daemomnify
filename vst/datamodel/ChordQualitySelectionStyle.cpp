#include "ChordQualitySelectionStyle.h"

#include <stdexcept>

ButtonPerChordQuality::ButtonPerChordQuality(std::map<int, ChordQuality> notes, std::map<int, ChordQuality> ccs)
    : notes(std::move(notes)), ccs(std::move(ccs)) {}

CCRangePerChordQuality::CCRangePerChordQuality(int cc) : cc(cc) {}

ChordQualitySelectionStyle::ChordQualitySelectionStyle(ButtonPerChordQuality v) : value(std::move(v)) {}

ChordQualitySelectionStyle::ChordQualitySelectionStyle(CCRangePerChordQuality v) : value(std::move(v)) {}

nlohmann::json ChordQualitySelectionStyle::to_json() const {
    nlohmann::json j;
    std::visit(
        [&j](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;
            j = arg;
            if constexpr (std::is_same_v<T, ButtonPerChordQuality>) {
                j["type"] = "ButtonPerChordQuality";
            } else if constexpr (std::is_same_v<T, CCRangePerChordQuality>) {
                j["type"] = "CCRangePerChordQuality";
            }
        },
        value);
    return j;
}

ChordQualitySelectionStyle ChordQualitySelectionStyle::from_json(const nlohmann::json& j) {
    std::string type = j.at("type").get<std::string>();

    if (type == "ButtonPerChordQuality") {
        return ChordQualitySelectionStyle(j.get<ButtonPerChordQuality>());
    } else if (type == "CCRangePerChordQuality") {
        return ChordQualitySelectionStyle(j.get<CCRangePerChordQuality>());
    }

    throw std::runtime_error("Unknown ChordQualitySelectionStyle type: " + type);
}
