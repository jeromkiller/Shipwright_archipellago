#pragma once
#ifdef __cplusplus

#include <unordered_map>

namespace AP_Text {
enum class TextColor : char {
    COLOR_DEFAULT = 0,
    COLOR_ERROR,
    COLOR_LOG,
    COLOR_BLACK,
    COLOR_RED,
    COLOR_GREEN,
    COLOR_YELLOW,
    COLOR_BLUE,
    COLOR_CYAN,
    COLOR_MAGENTA,
    COLOR_SLATEBLUE,
    COLOR_PLUM,
    COLOR_SALMON,
    COLOR_WHITE,
    COLOR_ORANGE,
    COLOR_GRAY
};

static std::unordered_map<AP_Text::TextColor, const ImVec4> colorVec = {
    { AP_Text::TextColor::COLOR_ERROR, ImVec4(1.0f, 0.4f, 0.4f, 1.0f) },
    { AP_Text::TextColor::COLOR_LOG, ImVec4(0.7f, 0.7f, 1.0f, 1.0f) },
    { AP_Text::TextColor::COLOR_BLACK, ImVec4(0.000f, 0.000f, 0.000f, 1.00f) },
    { AP_Text::TextColor::COLOR_RED, ImVec4(0.933f, 0.000f, 0.000f, 1.00f) },
    { AP_Text::TextColor::COLOR_GREEN, ImVec4(0.000f, 1.000f, 0.498f, 1.00f) },
    { AP_Text::TextColor::COLOR_YELLOW, ImVec4(0.980f, 0.980f, 0.824f, 1.00f) },
    { AP_Text::TextColor::COLOR_BLUE, ImVec4(0.392f, 0.584f, 0.929f, 1.00f) },
    { AP_Text::TextColor::COLOR_CYAN, ImVec4(0.000f, 0.933f, 0.933f, 1.00f) },
    { AP_Text::TextColor::COLOR_MAGENTA, ImVec4(0.933f, 0.000f, 0.933f, 1.00f) },
    { AP_Text::TextColor::COLOR_SLATEBLUE, ImVec4(0.427f, 0.545f, 0.910f, 1.00f) },
    { AP_Text::TextColor::COLOR_PLUM, ImVec4(0.686f, 0.600f, 0.937f, 1.00f) },
    { AP_Text::TextColor::COLOR_SALMON, ImVec4(0.980f, 0.502f, 0.447f, 1.00f) },
    { AP_Text::TextColor::COLOR_ORANGE, ImVec4(1.000, 0.467f, 0.000f, 1.000f) },
    { AP_Text::TextColor::COLOR_GRAY, ImVec4(0.53f, 0.53f, 0.53f, 1.00f) },
    { AP_Text::TextColor::COLOR_WHITE, ImVec4(0.93f, 0.93f, 0.93f, 1.00f) },
    { AP_Text::TextColor::COLOR_DEFAULT, ImVec4(0.93f, 0.93f, 0.93f, 1.00f) }
};

struct ColoredTextNode {
    std::string text;
    AP_Text::TextColor color;
};
}; // namespace AP_Text

namespace AP_Hint {
enum class HintStatus : char {
    HINT_UNSPECIFIED = 0,
    HINT_NO_PRIORITY,
    HINT_AVOID,
    HINT_PRIORITY,
    HINT_FOUND
};

enum AP_Item_Flags {
    FLAG_NONE = 0,
    FLAG_ADVANCEMENT = 1,
    FLAG_NEVER_EXCLUDE = 2,
    FLAG_TRAP = 4,
};

static std::unordered_map<AP_Hint::HintStatus, const std::string> statusStrings = { { AP_Hint::HintStatus::HINT_UNSPECIFIED, "Unspecified" },
                                                                     { AP_Hint::HintStatus::HINT_NO_PRIORITY, "No Priority" },
                                                                     { AP_Hint::HintStatus::HINT_AVOID, "Avoid" },
                                                                     { AP_Hint::HintStatus::HINT_PRIORITY, "Priority" },
                                                                     { AP_Hint::HintStatus::HINT_FOUND, "Found" } };

struct Hint {
    uint64_t index;
    std::string receiving_player_name;
    std::string finding_player_name;
    std::string location_name;
    std::string item_name;
    std::string entrance_name;
    AP_Hint::HintStatus hint_status;
    int item_flags;
    int finding_player_id;
    int location_id;
    bool found;
    bool we_find;
    bool we_receive;
};

}; // namespace AP_Hint

#endif