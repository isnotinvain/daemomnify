#include "ChordQualitySelectionStyle.h"

#include <stdexcept>

ButtonPerChordQuality::ButtonPerChordQuality(std::map<int, ChordQuality> notes, std::map<int, ChordQuality> ccs)
    : notes(std::move(notes)), ccs(std::move(ccs)) {}

CCRangePerChordQuality::CCRangePerChordQuality(int cc) : cc(cc) {}

ChordQualitySelectionStyle::ChordQualitySelectionStyle(ButtonPerChordQuality v) : value(std::move(v)) {}

ChordQualitySelectionStyle::ChordQualitySelectionStyle(CCRangePerChordQuality v) : value(v) {}

void to_json(nlohmann::json& j, const ChordQualitySelectionStyle& style) {
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
        style.value);
}

void from_json(const nlohmann::json& j, ChordQualitySelectionStyle& style) {
    std::string type = j.at("type").get<std::string>();

    if (type == "ButtonPerChordQuality") {
        style = ChordQualitySelectionStyle(j.get<ButtonPerChordQuality>());
    } else if (type == "CCRangePerChordQuality") {
        style = ChordQualitySelectionStyle(j.get<CCRangePerChordQuality>());
    } else {
        throw std::runtime_error("Unknown ChordQualitySelectionStyle type: " + type);
    }
}
