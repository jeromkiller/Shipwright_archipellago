#pragma once
#ifndef ARCHIPELAGO_HINT_WINDOW_H
#define ARCHIPELAGO_HINT_WINDOW_H

#include <libultraship/libultraship.h>
#include <vector>
#include "ship/window/gui/Gui.h"
#include "ArchipelagoTypes.h"

class ArchipelagoHintWindow final : public Ship::GuiWindow {
  public:
    using GuiWindow::GuiWindow;
    ~ArchipelagoHintWindow(){};

  protected:
    void InitElement() override{};
    void DrawElement() override;
    void UpdateElement() override{};

  private:
    void addName(const std::string& name, bool is_us);
    void addItem(const AP_Hint::Hint& hint);
    void addLocation(const AP_Hint::Hint& hint);
    void addEntrance(const AP_Hint::Hint& hint);
    void addStatus(const AP_Hint::Hint& hint);
    void addStatusCombo(const AP_Hint::Hint& hint);
    AP_Text::TextColor getStatusColor(const AP_Hint::HintStatus status) const;
};

void ArchipelagoHintWindow_UpdateHints(std::vector<AP_Hint::Hint>& new_hints);

#endif // ARCHIPELAGO_HINT_WINDOW_H