#include "file_browser_gui.h"
#include "editor/editor_gui.h"
#include "core/meta/registration.h"
#include "editor/gui/imgui_renderer.h"

static bool contains_any_folder(const fs::path& path) {
  return std::any_of(
      fs::directory_iterator(path),
      fs::directory_iterator(),
      [](auto& e){ return e.is_directory(); }
    );
}

static fs::path get_path(fs::path::iterator first, fs::path::iterator last) {
  fs::path path;
  for (auto it = first; it != last; it++) {
    path /= *it;
  }
  return path;
}

class file_browser_gui : public editor_gui {
 private:
  fs::path current_dir_ = {};
  fs::path popup_dir_;

 private:
  void draw_directory_content() {
    gui::BeginChild("##directory_content", { }, false);

    int i = 0;
    const float max_column_width = 80.0f;
    const float spacing = 4.0f;
    int columns = gui::GetContentRegionAvail().x / max_column_width;
    float width = (gui::GetContentRegionAvail().x - spacing * (columns-1)) / columns;
    const ImVec2 size { width, width };
    for (auto it = fs::directory_iterator(fs::absolute(current_dir_)); it != fs::directory_iterator(); it++) {
      if (i % columns) gui::SameLine(0.0f , spacing);

      if (gui::Button(it->path().filename().c_str(), size)) {

      }

      i++;
    }
    gui::EndChild();
  }

  void draw_directories_tree() {

    const float spacing = 4.0f;
    int recursion_depth = 0;

    for (auto it = fs::recursive_directory_iterator(fs::paths::project(), fs::directory_options::skip_permission_denied); it != fs::recursive_directory_iterator(); it++) {

      while (recursion_depth > it.depth()) {
        gui::TreePop();
        recursion_depth--;
      }

      assert(recursion_depth == it.depth());
//      recursion_depth = it.depth();

      if (!it->is_directory()) {
        continue;
      }

      bool leaf = !contains_any_folder(it->path());
      bool selected = it->path() == fs::absolute(current_dir_);

      ImGuiTreeNodeFlags flag =
          ImGuiTreeNodeFlags_SpanAvailWidth
          | ImGuiTreeNodeFlags_SpanFullWidth
          | ImGuiTreeNodeFlags_AllowItemOverlap
          | ImGuiTreeNodeFlags_OpenOnArrow
          | ImGuiTreeNodeFlags_FramePadding
          ;

      if (leaf) flag |= ImGuiTreeNodeFlags_Leaf;
      if (selected) flag |= ImGuiTreeNodeFlags_Selected;

      gui::PushStyleVar(ImGuiStyleVar_ItemSpacing, {0, spacing });
      bool open = ImGui::TreeNodeEx(it->path().c_str(), flag, it->path().filename().c_str(), "");
      gui::PopStyleVar();

      if (open) {
        recursion_depth++;
      }

      if (!open || leaf) {
        it.disable_recursion_pending();
      }
    }

    while (recursion_depth--) {
        gui::TreePop();
    }
  }

  void draw_select_panel() {
//    gui::PushStyleColor(ImGuiCol_Button, ImGui::GetColorU32(ImGuiCol_WindowBg));
//    gui::PushStyleColor(ImGuiCol_ButtonActive, ImGui::GetColorU32(ImGuiCol_WindowBg));
//    gui::PushStyleColor(ImGuiCol_ButtonHovered, ImGui::GetColorU32(ImGuiCol_WindowBg));
    const float spacing = 2.0f;

    // TODO: project name
    if (gui::Button("/")) {
      current_dir_.clear();
    }


    gui::SameLine(0, spacing);
    if (gui::ArrowButton("##arrow", ImGuiDir_Right)) {
      popup_dir_ = fs::paths::project();
      ImGui::OpenPopup("my_file_popup");
    }

    uint32_t counter = 0;

    fs::path new_current;
    for (auto it = current_dir_.begin(); it != current_dir_.end(); it++, counter++) {

      char str_btn_id[16] = "btn";
      char str_arr_id[16] = "arr";
      sprintf(str_btn_id, "%u", counter);
      sprintf(str_arr_id, "%u", counter);

      gui::SameLine(0, spacing);
      gui::PushID(str_btn_id);
      if (gui::Button(it->c_str())) {
        new_current = get_path(current_dir_.begin(), std::next(it));
      }
      gui::PopID();

      gui::SameLine(0, spacing);
      if (gui::ArrowButton(str_arr_id, ImGuiDir_Right)) {
        popup_dir_ = fs::absolute(get_path(current_dir_.begin(), std::next(it)));
        if (contains_any_folder(popup_dir_)) {
          ImGui::OpenPopup("my_file_popup");
        }
      }
    }

//    gui::PopStyleColor(3);

    if (ImGui::BeginPopup("my_file_popup")) {
      select_popup();
      ImGui::EndPopup();
    } else if (!new_current.empty()) {
      current_dir_ = new_current;
    }
  }

  void select_popup() {

    if (!fs::exists(popup_dir_)) {
      return;
    }

    for (const auto& entry : fs::directory_iterator(popup_dir_)) {
      if (!entry.is_directory())
        continue;

      if (ImGui::MenuItem(entry.path().filename().c_str())) {
        current_dir_ = fs::relative(entry.path(), fs::paths::project());
      }
    }
  }

 public:
  void start(assets::repository*) override {}

  void gui(gui_renderer* gui_renderer) override {
    gui_renderer->set_context();

    if (gui::GetCurrentContext()) {
      gui::Begin("Project Browser");

      draw_select_panel();

      ImGui::Columns(2, "word-wrapping", true);

      float w = gui::GetColumnWidth(0) - gui::GetCursorPosX();
      float child_w = w;//(ImGui::GetContentRegionAvail().x - w);
      float child_h = gui::GetWindowHeight() - gui::GetCursorPosY();

      ImGuiWindowFlags child_flags = ImGuiWindowFlags_None;// | ImGuiWindowFlags_NoBackground;

//      gui::PushStyleColor(ImGuiCol_ChildBg, ImGui::GetColorU32(ImGuiCol_Button));


      ImGui::BeginChild("child_id", { }, false, child_flags);
//      gui::PopStyleColor(1);


      draw_directories_tree();

      ImGui::EndChild();

      ImGui::NextColumn();

      draw_directory_content();

      ImGui::Columns(1);

      gui::End();
    }
  }
};

void load_file_browser_gui(plugins*) {
  register_type(file_browser_gui);

  editor::g_editor_gui->add_editor<file_browser_gui>();
}

void unload_file_browser_gui(plugins*) {
  editor::g_editor_gui->remove_editor<file_browser_gui>();
}