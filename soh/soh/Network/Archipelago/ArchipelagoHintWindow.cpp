#include "ArchipelagoHintWindow.h"

#include "soh/SohGui/UIWidgets.hpp"
#include "soh/SohGui/SohGui.hpp"
#include "soh/OTRGlobals.h"
#include "ArchipelagoTypes.h"
#include "Archipelago.h"
#include <apclient.hpp>

std::vector<AP_Hint::Hint> HintList;
bool hints_updated = false;

using namespace UIWidgets;

void ArchipelagoHintWindow::DrawElement() {
    ImGui::SeparatorText("Archipelago Hints");

    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.15f, 0.15f, 0.15f, 1.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 8.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(15.0f, 12.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 1.0f));

    static ImGuiTableFlags flags = ImGuiTableFlags_Resizable | ImGuiTableFlags_Sortable |
                                          ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter |
                                          ImGuiTableFlags_BordersV | ImGuiTableFlags_NoBordersInBody |
                                          ImGuiTableFlags_ScrollY;
    if (ImGui::BeginTable("archipelago_hint_table", 5, flags)) {
        // headers
        ImGui::TableSetupColumn("Receiving Player");
        ImGui::TableSetupColumn("Item");
        ImGui::TableSetupColumn("Finding Player");
        ImGui::TableSetupColumn("Location");
        ImGui::TableSetupColumn("Status", ImGuiTableColumnFlags_DefaultSort);
        ImGui::TableHeadersRow();

        for (const AP_Hint::Hint& hint : HintList) {
            ImGui::PushID(hint.index);
            addName(hint.receiving_player_name, hint.we_receive);
            addItem(hint);
            addName(hint.finding_player_name, hint.we_find);
            addLocation(hint);
            addStatus(hint);
            ImGui::PopID();
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
    ImGui::PushStyleColor(ImGuiCol_Text, AP_Text::colorVec[color]);    // todo find out color based on flags
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
    const std::array<AP_Hint::HintStatus, 3> drop_down_statuses = {AP_Hint::HintStatus::AVOID, AP_Hint::HintStatus::NO_PRIORITY, AP_Hint::HintStatus::PRIORITY};

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

void ArchipelagoHintWindow_UpdateHints(std::vector<AP_Hint::Hint>& new_hints) {
    HintList.swap(new_hints);
    hints_updated = true;
}