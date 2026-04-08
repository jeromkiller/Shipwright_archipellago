#include "ArchipelagoHintWindow.h"

#include "soh/SohGui/UIWidgets.hpp"
#include "soh/SohGui/SohGui.hpp"
#include "soh/OTRGlobals.h"
#include "ArchipelagoTypes.h"
#include "Archipelago.h"
#include <apclient.hpp>
#include <unordered_set>

#include "ArchipelagoConsoleWindow.h"
#include "ItemSuggestionTrie.h"

std::vector<AP_Hint::Hint> HintList;
ItemSuggestionTrie suggestionTrie;
bool hints_updated = false;

using namespace UIWidgets;

void ArchipelagoHintWindow::DrawElement() {
    //ImGui::SeparatorText("Archipelago Hints");

    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.15f, 0.15f, 0.15f, 1.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(15.0f, 12.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 1.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 0.0f);

    UIWidgets::ButtonOptions sendButtonOptions = UIWidgets::ButtonOptions().Color(THEME_COLOR).Size(ImVec2(0.0, 0.0));
    int HintInputHeight = ImGui::GetTextLineHeightWithSpacing() * 2;

    static ImGuiTableFlags flags = ImGuiTableFlags_Resizable | ImGuiTableFlags_Sortable | ImGuiTableFlags_SortMulti |
                                   ImGuiTableFlags_SortTristate | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuterH |
                                   ImGuiTableFlags_BordersV | ImGuiTableFlags_NoBordersInBody | ImGuiTableFlags_ScrollY;

    if (ImGui::BeginTable("archipelago_hint_table", 5, flags, ImVec2(0.0f, -HintInputHeight - 5))) {
        // headers
        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableSetupColumn("Receiving Player", 0, 0.0f, HintTableColumns::COL_RECIEVING);
        ImGui::TableSetupColumn("Item", 0, 0.0f, HintTableColumns::COL_ITEM);
        ImGui::TableSetupColumn("Finding Player", 0, 0.0f, HintTableColumns::COL_FINDING);
        ImGui::TableSetupColumn("Location", 0, 0.0f, HintTableColumns::COL_LOCATION);
        ImGui::TableSetupColumn("Status", 0, 0.0f, HintTableColumns::COL_STATUS);
        ImGui::TableHeadersRow();

        sortHints(ImGui::TableGetSortSpecs());

        // content
        for (const AP_Hint::Hint& hint : HintList) {
            ImGui::PushID(static_cast<int>(hint.index));
            addName(hint.receiving_player_name, hint.we_receive);
            addItem(hint);
            addName(hint.finding_player_name, hint.we_find);
            addLocation(hint);
            addStatus(hint);
            ImGui::PopID();
        }

        ImGui::EndTable();
    }

    static char textEntryBuf[50];
    // Ideally I don't want this to be a text field that shows a child window instead of a combo box,
    // https://github.com/ocornut/imgui/issues/718 has some more exotic methods of achieving this
    // But something like this might be slated for a future version of ImGui and this is good enough for now
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.0, 0.0, 0.0, 0.0));
    ImGui::BeginChild("HintBoxLeft", ImVec2(-HintInputHeight * 3.5, 0.0f));
    ImGui::PopStyleColor();

    int chatbarHeight = ImGui::GetTextLineHeight() + ImGui::GetStyle().ItemSpacing.x
        + sendButtonOptions.padding.y
        + 5.0f * 2.0f; // FrameBorderSize * 2
    float chatBarPadding = (ImGui::GetWindowHeight() - chatbarHeight) / 2.0f;
    if (chatBarPadding > 0.0f) {
        ImGui::Dummy(ImVec2(0.0f, chatBarPadding));
    }

    PushStyleInput(THEME_COLOR);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10.0f, 8.0f));
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 3.0);
    if (ImGui::BeginCombo("##HintCombo", "New Hint", 0)) {
        if (ImGui::IsWindowAppearing()) {
            ImGui::SetKeyboardFocusHere();
            textEntryBuf[0] = '\0';
        }
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        ImGui::InputText("##AP_HintEntryField", textEntryBuf, 49);
        auto hintView = std::string_view(textEntryBuf);
        const std::unordered_set<int64_t> suggestions = suggestionTrie.GetSuggestions(hintView);
        std::vector<std::string> SuggestedItems;
        for (int ApItemId : suggestions) {
            SuggestedItems.emplace_back(ArchipelagoClient::GetInstance().GetApItemName(ApItemId));
        }
        std::sort(SuggestedItems.begin(), SuggestedItems.end());

        for (int i = 0; i < SuggestedItems.size(); i++) {
            ImGui::PushID(i);
            std::string SuggestedItem = SuggestedItems[i];
            if (ImGui::Selectable(SuggestedItem.c_str())) {
                ArchipelagoClient::GetInstance().SendMessageToConsole("!hint '" + SuggestedItem + "'");
            }
            ImGui::PopID();
        }
        if (suggestions.empty()) {
            ImGui::TextColored(AP_Text::colorVec[AP_Text::TextColor::COLOR_RED], "No matching items found");
        }
        ImGui::EndCombo();
    }
    ImGui::PopStyleVar();
    PopStyleInput();
    ImGui::EndChild();

    ImGui::SameLine();

    if (ImGui::BeginTable("ScoreTable", 2)) {
        ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthFixed, ImGui::CalcTextSize("Hint Points:").x);
        ImGui::TableSetupColumn("Value");

        const int hintCost = ArchipelagoClient::GetInstance().GetHintCost();
        const int hintPoints = ArchipelagoClient::GetInstance().GetHintPoints();

        // Todo I'd like the points to be right alligned, but It looks like Omar is still working on that
        ImGui::TableNextColumn();
        ImGui::Text("Hint Cost:");
        ImGui::TableNextColumn();
        ImGui::Text("%d", hintCost);

        ImGui::TableNextColumn();
        ImGui::Text("Hint Points:");
        ImGui::TableNextColumn();
        if (hintPoints < hintCost) {
            ImGui::TextColored(AP_Text::colorVec[AP_Text::TextColor::COLOR_SALMON], "%d", hintPoints);
        } else {
            ImGui::Text("%d", hintPoints);
        }
        ImGui::EndTable();
    }

    ImGui::PopStyleColor();
    ImGui::PopStyleVar(3);
}

void ArchipelagoHintWindow::addName(const std::string& name, bool is_us) {
    ImGui::TableNextColumn();
    if (is_us) {
        ImGui::PushStyleColor(ImGuiCol_Text, AP_Text::colorVec[AP_Text::TextColor::COLOR_MAGENTA]);
    } else {
        ImGui::PushStyleColor(ImGuiCol_Text, AP_Text::colorVec[AP_Text::TextColor::COLOR_YELLOW]);
    }
    ImGui::TextWrapped("%s", name.c_str());
    ImGui::PopStyleColor();
}

void ArchipelagoHintWindow::addItem(const AP_Hint::Hint& hint) {
    ImGui::TableNextColumn();
    AP_Text::TextColor color = AP_Text::TextColor::COLOR_CYAN;
    if (hint.item_flags & APClient::ItemFlags::FLAG_ADVANCEMENT)
        color = AP_Text::TextColor::COLOR_PLUM;
    else if (hint.item_flags & APClient::ItemFlags::FLAG_NEVER_EXCLUDE)
        color = AP_Text::TextColor::COLOR_SLATEBLUE;
    else if (hint.item_flags & APClient::ItemFlags::FLAG_TRAP)
        color = AP_Text::TextColor::COLOR_SALMON;
    ImGui::PushStyleColor(ImGuiCol_Text, AP_Text::colorVec[color]);
    ImGui::TextWrapped("%s", hint.item_name.c_str());
    ImGui::PopStyleColor();
}

void ArchipelagoHintWindow::addLocation(const AP_Hint::Hint& hint) {
    ImGui::TableNextColumn();
    ImGui::PushStyleColor(ImGuiCol_Text, AP_Text::colorVec[AP_Text::TextColor::COLOR_GREEN]);
    ImGui::TextWrapped("%s", hint.location_name.c_str());
    ImGui::PopStyleColor();
}

void ArchipelagoHintWindow::addEntrance(const AP_Hint::Hint& hint) {
    ImGui::TableNextColumn();
    ImGui::PushStyleColor(ImGuiCol_Text, AP_Text::colorVec[AP_Text::TextColor::COLOR_BLUE]);
    ImGui::TextWrapped("%s", hint.entrance_name.c_str());
    ImGui::PopStyleColor();
}

void ArchipelagoHintWindow::addStatus(const AP_Hint::Hint& hint) {
    ImGui::TableNextColumn();
    ImGui::PushItemWidth(-FLT_MIN);
    AP_Text::TextColor color = getStatusColor(hint.hint_status);
    ImGui::PushStyleColor(ImGuiCol_Text, AP_Text::colorVec[color]);
    if (hint.found || !hint.we_receive) {
        ImGui::TextWrapped("%s", AP_Hint::statusStrings[hint.hint_status].c_str());
    } else {
        addStatusCombo(hint);
    }

    ImGui::PopItemWidth();
    ImGui::PopStyleColor();
}

void ArchipelagoHintWindow::addStatusCombo(const AP_Hint::Hint& hint) {
    const std::array<AP_Hint::HintStatus, 3> drop_down_statuses = { AP_Hint::HintStatus::AVOID,
                                                                    AP_Hint::HintStatus::NO_PRIORITY,
                                                                    AP_Hint::HintStatus::PRIORITY };

    // set up combo box style
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.0, 0.0, 0.0, 0.0));
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(1.0, 1.0, 1.0, 0.1));
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0, 0.0, 0.0, 0.0));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0, 1.0, 1.0, 0.1));
    ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(1.0, 1.0, 1.0, 0.1));
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(1.0, 1.0, 1.0, 0.2));

    if (ImGui::BeginCombo("", AP_Hint::statusStrings[hint.hint_status].c_str(), 0)) {
        for (const AP_Hint::HintStatus status : drop_down_statuses) {
            ImGui::PushStyleColor(ImGuiCol_Text, AP_Text::colorVec[getStatusColor(status)]);
            const bool is_selected = hint.hint_status == status;
            if (ImGui::Selectable(AP_Hint::statusStrings[status].c_str(), is_selected)) {
                // update hint status
                ArchipelagoClient::GetInstance().UpdateHintStatus(hint.finding_player_id, hint.location_id, status);
            }

            if (is_selected)
                ImGui::SetItemDefaultFocus();

            ImGui::PopStyleColor();
        }

        ImGui::EndCombo();
    }
    ImGui::PopStyleColor(6);
}

AP_Text::TextColor ArchipelagoHintWindow::getStatusColor(const AP_Hint::HintStatus status) const {
    switch (status) {
        case AP_Hint::HintStatus::FOUND:
            return AP_Text::TextColor::COLOR_GREEN;
        case AP_Hint::HintStatus::NO_PRIORITY:
            return AP_Text::TextColor::COLOR_CYAN;
        case AP_Hint::HintStatus::AVOID:
            return AP_Text::TextColor::COLOR_SALMON;
        case AP_Hint::HintStatus::PRIORITY:
            return AP_Text::TextColor::COLOR_PLUM;
        case AP_Hint::HintStatus::UNSPECIFIED:
            return AP_Text::TextColor::COLOR_DEFAULT;
    }
    return AP_Text::TextColor::COLOR_ERROR;
}

// Sort the hintlist using the stl sort
// multi column sorting method coppied from https://pthom.github.io/imgui_explorer/ Line: 5845, func
// CompareWithSortSpecs
void ArchipelagoHintWindow::sortHints(ImGuiTableSortSpecs* sort_specs) {
    if (sort_specs == NULL) {
        return;
    }

    if (!sort_specs->SpecsDirty && !hints_updated) {
        return;
    }

    if (HintList.size() <= 1) {
        return;
    }

    std::sort(HintList.begin(), HintList.end(), [&](const AP_Hint::Hint& lhs, const AP_Hint::Hint& rhs) {
        for (int i = 0; i < sort_specs->SpecsCount; i++) {
            const ImGuiTableColumnSortSpecs* spec = &sort_specs->Specs[i];
            int delta = 0;
            switch (spec->ColumnUserID) {
                case COL_RECIEVING:
                    delta = lhs.receiving_player_name.compare(rhs.receiving_player_name);
                    break;
                case COL_ITEM:
                    delta = lhs.item_name.compare(rhs.item_name);
                    break;
                case COL_FINDING:
                    delta = lhs.finding_player_name.compare(rhs.finding_player_name);
                    break;
                case COL_LOCATION:
                    delta = lhs.location_name.compare(rhs.location_name);
                    break;
                case COL_STATUS:
                    delta = static_cast<char>(lhs.hint_status) - static_cast<char>(rhs.hint_status);
                    break;
            }

            if (delta < 0) {
                return spec->SortDirection == ImGuiSortDirection_Ascending;
            } else if (delta > 0) {
                return spec->SortDirection != ImGuiSortDirection_Ascending;
            }
        }

        // no sort / explicit difference, return order based on index
        return lhs.index < rhs.index;
    });

    sort_specs->SpecsDirty = false;
    hints_updated = false;
}

void ArchipelagoHintWindow_UpdateHints(std::vector<AP_Hint::Hint>& new_hints) {
    HintList.swap(new_hints);
    hints_updated = true;
}

void ArchipelagoHintWindow_ChangeHintableItems(const std::vector<int64_t>& hintableItems) {
    suggestionTrie.Clear();
    for (const int64_t itemId : hintableItems) {
        std::string ItemName = ArchipelagoClient::GetInstance().GetApItemName(itemId);
        suggestionTrie.AddItem(ItemName, itemId);
    }
}

void ArchipelagoHintWindow_ClearItemSuggestions() {
    suggestionTrie.Clear();
}