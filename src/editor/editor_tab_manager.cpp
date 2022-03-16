#include "editor_tab_manager.h"

#include "editor_tab.h"
#include "core/input_system.h"

#include <imgui.h>
#include <imgui_internal.h>

void begin_dockspace() {
  static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;
  ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;

  ImGuiViewport* viewport = ImGui::GetMainViewport();
  ImGui::SetNextWindowPos(viewport->GetWorkPos());
  ImGui::SetNextWindowSize(viewport->GetWorkSize());
  ImGui::SetNextWindowViewport(viewport->ID);

  ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

  window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
  window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

  ImGui::Begin("DockSpace", nullptr, window_flags);

  ImGui::PopStyleVar(3);

  {
    // tools panel
    ImGui::RenderFrame(
        ImGui::GetCursorScreenPos(),
        {
            ImGui::GetCursorScreenPos().x + ImGui::GetWindowWidth(), ImGui::GetCursorScreenPos().y + ImGui::GetFrameHeight()
        }, ImGui::GetColorU32(ImGuiCol_DockingEmptyBg));

    ImGui::NewLine();
  }


  ImGuiIO& io = ImGui::GetIO();
  ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
  ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);


  if (ImGui::BeginMenuBar()) {
    if (ImGui::BeginMenu("File")) {
      if (ImGui::MenuItem("Close", NULL, false, true)) {}
      ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Edit")) {
      ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("View")) {
      ImGui::EndMenu();
    }

    ImGui::EndMenuBar();
  }

  ImGui::End();
}

editor_tab_manager::editor_tab_manager(systems_registry& registry)
  : editor_tab_view_(registry.view<editor_tab>())
  , gui_renderer_(registry.get<gui>())
  , input_system_(registry.get<input_system>())
{
  input_system_->on_drop.connect(*this, &editor_tab_manager::on_drop);
}

void editor_tab_manager::update() {
  begin_dockspace();

  for (auto& tab : editor_tab_view_) {
    tab->begin(gui_renderer_.get());

    if (!drop_paths_.empty() && tab->is_hovered()) {
      tab->on_drop(drop_paths_);
      drop_paths_.resize(0);
    }

    tab->gui();
    tab->end();
  }
  drop_paths_.resize(0);
}

editor_tab_manager::~editor_tab_manager() {
  input_system_->on_drop.disconnect(*this, &editor_tab_manager::on_drop);
}

void editor_tab_manager::on_drop(const drop_event& e) {
  for (auto& p : e.paths) {
    drop_paths_.push_back(p);
  }
}
