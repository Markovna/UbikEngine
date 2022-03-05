#include "editor_tab.h"
#include "imgui.h"

void editor_tab::begin(::gui* g) const {
  g->set_context();
  ImGui::Begin(name().data());
};

void editor_tab::end() const {
  ImGui::End();
};