#include "editor_tab.h"
#include "imgui.h"
#include "base/window_event.h"

void editor_tab::begin(::gui* g) const {
  g->set_context();
  ImGui::Begin(name().data());
};

void editor_tab::end() const {
  ImGui::End();
}

bool editor_tab::is_hovered() const {
  return ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows | ImGuiHoveredFlags_AllowWhenBlockedByPopup | ImGuiHoveredFlags_AllowWhenBlockedByActiveItem);
};