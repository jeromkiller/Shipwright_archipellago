#include "ArchipelagoConsoleWindow.h"

#include "soh/SohGui/UIWidgets.hpp"
#include "soh/SohGui/SohGui.hpp"
#include "soh/OTRGlobals.h"
#include "ArchipelagoTypes.h"

std::vector<std::vector<AP_Text::ColoredTextNode>> Items;
bool autoScroll = true;

using namespace UIWidgets;

void ArchipelagoConsole_SendMessage(const char* fmt, ...) {
    char buf[1024];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, IM_ARRAYSIZE(buf), fmt, args);
    buf[IM_ARRAYSIZE(buf) - 1] = 0;
    va_end(args);
    AP_Text::ColoredTextNode node;
    node.text = std::string(buf);
    node.color = AP_Text::TextColor::COLOR_WHITE;
    if (strstr(buf, "[ERROR]")) {
        node.color = AP_Text::TextColor::COLOR_ERROR;
    } else if (strstr(buf, "[LOG]")) {
        node.color = AP_Text::TextColor::COLOR_LOG;
    }
    std::vector<AP_Text::ColoredTextNode> line;
    line.push_back(node);
    Items.push_back(line);
    if (Items.size() > 50) {
        Items.erase(Items.begin());
    }
}

void ArchipelagoConsole_PrintJson(const std::vector<AP_Text::ColoredTextNode> nodes) {
    Items.push_back(nodes);
    if (Items.size() > 50) {
        Items.erase(Items.begin());
    }
}

void ArchipelagoConsoleWindow::DrawElement() {
    ImGui::SeparatorText("Archipelago Log");

    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.15f, 0.15f, 0.15f, 1.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 8.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(15.0f, 12.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 1.0f));

    UIWidgets::ButtonOptions sendButtonOptions = UIWidgets::ButtonOptions().Color(THEME_COLOR).Size(ImVec2(0.0, 0.0));
    int chatbarHeight = ImGui::GetTextLineHeight() + ImGui::GetStyle().ItemSpacing.x
        + sendButtonOptions.padding.y
        + 5.0f * 2.0f; // FrameBorderSize * 2

    if (ImGui::BeginChild("ScrollingRegion", ImVec2(0, -chatbarHeight), ImGuiChildFlags_AlwaysUseWindowPadding,
                          ImGuiWindowFlags_HorizontalScrollbar)) {

        for (const std::vector<AP_Text::ColoredTextNode>& line : Items) {
            for (const AP_Text::ColoredTextNode& node : line) {
                ImGui::PushStyleColor(ImGuiCol_Text, AP_Text::colorVec[node.color]);
                ImGui::TextUnformatted(node.text.c_str());
                ImGui::SameLine();
                ImGui::PopStyleColor();
            }
            ImGui::NewLine();
        }

        // Keep up at the bottom of the scroll region if we were already at the bottom at the beginning of the frame.
        // Using a scrollbar or mouse-wheel will take away from the bottom edge.
        if (autoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
            ImGui::SetScrollHereY(1.0f);
        }
    }
    ImGui::EndChild();
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.0f);

    static char textEntryBuf[1024];
    static bool keepFocus = false;

    if (keepFocus) {
        ImGui::SetKeyboardFocusHere();
        keepFocus = false;
    }

    PushStyleInput(THEME_COLOR);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10.0f, 8.0f));
    if (ImGui::InputText("##AP_MessageField", textEntryBuf, 1023, ImGuiInputTextFlags_EnterReturnsTrue)) {
        ArchipelagoClient::GetInstance().SendMessageToConsole(std::string(textEntryBuf));
        textEntryBuf[0] = '\0';
        keepFocus = true;
    }
    ImGui::PopStyleVar();
    PopStyleInput();

    ImGui::SameLine();

    if (UIWidgets::Button("Send", sendButtonOptions)) {
        ArchipelagoClient::GetInstance().SendMessageToConsole(std::string(textEntryBuf));
        textEntryBuf[0] = '\0';
        keepFocus = true;
    }

    ImGui::PopStyleColor();
    ImGui::PopStyleVar(4);
};
