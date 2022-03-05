#include "core/schema.h"
#include "core/components/camera_component.h"
#include "core/render_pipeline.h"
#include "core/assets_filesystem.h"
#include "core/asset_repository.h"
#include "core/systems_registry.h"
#include "core/viewer_registry.h"
#include "core/viewport.h"
#include "core/renderer.h"
#include "editor_plugin.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "editor/editor_tab.h"
#include "core/world.h"
#include "platform/file_system.h"

static system_ptr<viewer_registry> viewer_registry;
static system_ptr<renderer> renderer;
static system_ptr<asset_repository> assets_repository;
static system_ptr<render_pipeline> render_pipeline;
static system_ptr<schema_registry> schema_registry;

class filebrowser_tab;
class entity_tree_tab;
class properties_tab;
class scene_tab;

static filebrowser_tab* filebrowser;
static entity_tree_tab* entity_tree;
static properties_tab* properties;
static scene_tab* scene;

class filebrowser_tab : public editor_tab {
 private:
  fs::path current_dir_;
  fs::path popup_dir_;
  fs::path selected_;

 private:
  static fs::path absolute(const fs::path& p) {
    if (p.empty()) return fs::project_path();
    fs::path current = fs::project_path();
    current.append(p.c_str());
    return current;
  }

  static fs::path get_path(fs::path::iterator first, fs::path::iterator last) {
    fs::path path;
    for (auto it = first; it != last; it++) {
      path /= *it;
    }
    return path;
  }

  static bool contains_any_folder(const fs::path& path) {
    return std::any_of(
        fs::directory_iterator(path),
        fs::directory_iterator(),
        [](auto& e){ return e.is_directory(); }
    );
  }

  void select_popup() {

    if (!fs::exists(popup_dir_)) {
      return;
    }

    for (const auto& entry : fs::directory_iterator(popup_dir_)) {
      if (!entry.is_directory())
        continue;

      if (ImGui::MenuItem(entry.path().filename().c_str())) {
        current_dir_ = fs::relative(entry.path(), fs::project_path());
      }
    }
  }

  void draw_directory_content() {
    ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_SelectableTextAlign, ImVec2(0, 0.5f));

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));      // Disable padding
    ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(30, 30, 30, 255));
    ImGui::BeginChild("##directory_content", { }, false);
    ImGui::PopStyleColor();
    ImGui::PopStyleVar();

    int i = 0;
    const float max_column_width = 180.0f;
    const float spacing = 4.0f;
    int columns = std::max(1, (int)(ImGui::GetContentRegionAvail().x / max_column_width));
    float width = (ImGui::GetContentRegionAvail().x - spacing * (columns-1)) / columns;
    const ImVec2 size { width, 20 };
    for (auto it = fs::directory_iterator(absolute(current_dir_)); it != fs::directory_iterator(); it++) {
      if (!it->is_directory() && !it->is_regular_file())
        continue;

      if (i % columns) ImGui::SameLine(0.0f , spacing);

      if (ImGui::Selectable(it->path().filename().c_str(), it->path() == selected_, ImGuiSelectableFlags_AllowDoubleClick | ImGuiSelectableFlags_NoPadWithHalfSpacing, size)) {
        selected_ = it->path();
      }

      if (it->is_directory() && ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {
        current_dir_ = fs::relative(it->path(), fs::project_path());
      }

      if (it->is_regular_file()) {
        if (ImGui::BeginDragDropSource(
            ImGuiDragDropFlags_AcceptNoPreviewTooltip |
                //ImGuiDragDropFlags_SourceNoHoldToOpenOthers |
                ImGuiDragDropFlags_AcceptNoDrawDefaultRect)) {

          ImGui::SetDragDropPayload("##file-dragdrop", it->path().c_str(), strlen(it->path().c_str()));
          ImGui::Text(it->path().filename().c_str(), "");
          ImGui::EndDragDropSource();
        }
      }

      i++;
    }

    ImGui::EndChild();
    ImGui::PopStyleVar(2);
  }

  bool draw_directory(const fs::path& path) {

    const float spacing = 0.0f;
    ImGuiTreeNodeFlags base_flags =
        ImGuiTreeNodeFlags_SpanAvailWidth
            | ImGuiTreeNodeFlags_SpanFullWidth
            | ImGuiTreeNodeFlags_AllowItemOverlap
            | ImGuiTreeNodeFlags_OpenOnArrow
            | ImGuiTreeNodeFlags_FramePadding
    ;

    ImGuiTreeNodeFlags flag = base_flags;

    bool leaf = !contains_any_folder(path);
    bool selected = path == absolute(current_dir_);

    if (leaf) flag |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
    if (selected) flag |= ImGuiTreeNodeFlags_Selected;

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, {0, spacing});
    bool open = ImGui::TreeNodeEx(path.c_str(), flag, path.filename().c_str(), "");
    ImGui::PopStyleVar();

    if (ImGui::IsItemClicked(ImGuiMouseButton_Left) &&
        !ImGui::IsItemToggledOpen()) {
      if (path == fs::project_path()) {
        current_dir_.clear();
      } else {
        current_dir_ = fs::relative(path, fs::project_path());
      }
    }

    return open && !leaf;
  }

  void draw_directories_tree() {
    ImGuiWindowFlags child_flags = ImGuiWindowFlags_None;// | ImGuiWindowFlags_NoBackground;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));      // Disable padding
    ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(30, 30, 30, 255));
    ImGui::BeginChild("child_id", { }, false, child_flags);
    ImGui::PopStyleColor();
    ImGui::PopStyleVar();

    int recursion_depth = 0;

    ImGuiTreeNodeFlags base_flags =
        ImGuiTreeNodeFlags_SpanAvailWidth
            | ImGuiTreeNodeFlags_SpanFullWidth
            | ImGuiTreeNodeFlags_AllowItemOverlap
            | ImGuiTreeNodeFlags_OpenOnArrow
            | ImGuiTreeNodeFlags_FramePadding
    ;

    bool base_open = draw_directory(fs::project_path());
    if (base_open) {
      for (
          auto it = fs::recursive_directory_iterator(fs::project_path(), fs::directory_options::skip_permission_denied);
          it != fs::recursive_directory_iterator();
          ++it) {

        while (recursion_depth > it.depth()) {
          ImGui::TreePop();
          recursion_depth--;
        }

        assert(recursion_depth == it.depth());

        if (!it->is_directory()) {
          continue;
        }

        if (!draw_directory(it->path())) {
          it.disable_recursion_pending();
        } else {
          recursion_depth++;
        }
      }

      while (recursion_depth--) {
        ImGui::TreePop();
      }

      ImGui::TreePop();
    }

    ImGui::EndChild();
  }

  void draw_select_panel() {
//    ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetColorU32(ImGuiCol_WindowBg));
//    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImGui::GetColorU32(ImGuiCol_WindowBg));
//    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImGui::GetColorU32(ImGuiCol_WindowBg));
    const float spacing = 2.0f;

    if (ImGui::Button(fs::project_path().filename().c_str())) {
      current_dir_ = { };
    }

    ImGui::SameLine(0, spacing);
    ImGui::PushID("##arrow");
    bool root_arrow = ImGui::ArrowButton("##arrow", ImGuiDir_Right);
    ImGui::PopID();
    if (root_arrow) {
      popup_dir_ = fs::project_path();
      ImGui::OpenPopup("my_file_popup");
    }

    uint32_t counter = 0;

    fs::path new_current;

    for (auto it = current_dir_.begin(); it != current_dir_.end(); it++, counter++) {

      char str_btn_id[16] = "btn";
      char str_arr_id[16] = "arr";
      sprintf(str_btn_id, "%u", counter);
      sprintf(str_arr_id, "%u", counter);

      ImGui::SameLine(0, spacing);
      ImGui::PushID(str_btn_id);
      if (ImGui::Button(it->c_str())) {
        new_current = get_path(current_dir_.begin(), std::next(it));
      }
      ImGui::PopID();

      ImGui::SameLine(0, spacing);
      ImGui::PushID(str_arr_id);
      bool arrow = ImGui::ArrowButton(str_arr_id, ImGuiDir_Right);
      ImGui::PopID();
      if (arrow) {
        popup_dir_ = absolute(get_path(current_dir_.begin(), std::next(it)));
        if (contains_any_folder(popup_dir_)) {
          ImGui::OpenPopup("my_file_popup");
        }
      }
    }

    if (ImGui::BeginPopup("my_file_popup")) {
      select_popup();
      ImGui::EndPopup();
    } else if (!new_current.empty()) {
      current_dir_ = new_current;
    }

//    ImGui::PopStyleColor(3);
  }

 public:
  [[nodiscard]] std::string_view name() const override {
    return "Asset Browser";
  }

  void gui() override {

    draw_select_panel();

    ImGui::PushStyleColor(ImGuiCol_Separator, ImGui::GetColorU32(ImGuiCol_WindowBg));
    ImGui::PushStyleColor(ImGuiCol_SeparatorHovered, ImGui::GetColorU32(ImGuiCol_WindowBg));
    ImGui::PushStyleColor(ImGuiCol_SeparatorActive, ImGui::GetColorU32(ImGuiCol_WindowBg));

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
    ImGui::Columns(2, "##filebrowser-columns", true);
    ImGui::PopStyleVar(3);

    float w = ImGui::GetColumnWidth(0) - ImGui::GetCursorPosX();
    float child_w = w;//(ImGui::GetContentRegionAvail().x - w);
    float child_h = ImGui::GetWindowHeight() - ImGui::GetCursorPosY();


    draw_directories_tree();

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(2, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
    ImGui::NextColumn();
    ImGui::PopStyleVar(3);

    draw_directory_content();

    ImGui::Columns(1);
    ImGui::PopStyleColor(3);
  }

  filebrowser_tab() : current_dir_(), popup_dir_(), selected_() {}
};

enum class visit_children_result {
  BREAK,
  CONTINUE,
  RECURSE
};

template<class Iterator, class Operation>
int32_t visit_children(Iterator first, Operation op, uint32_t depth = 0) {
  constexpr static const char* CHILDREN = "children";
  auto op_result = op(*first, depth);
  if (op_result == visit_children_result::BREAK)
    return -1;

  if (op_result == visit_children_result::CONTINUE)
    return 0;

  assert(op_result == visit_children_result::RECURSE);
  if (auto it = first->find(CHILDREN); it != first->end() && it->second.is_array()) {
    const asset_array& children = it->second;
    for (const asset& child : children) {
      if (auto result = visit_children(&child, op, depth + 1))
        return result;
    }
  }
  return 0;
}

class properties_tab : public editor_tab {
 public:
  [[nodiscard]] std::string_view name() const override {
    static const char* _name = "Properties";
    return _name;
  }

  void data_ui(const asset& asset, const std::string& name, const schema_property& property, uint32_t& id) {
    char label[16];
    sprintf(label,"%u", id);
    ImGui::PushID(label);

    switch (property.type) {
      case schema_type::FLOAT: {
        assert(asset.at(name).is_number_float());
        float value = asset.at(name);
        if (ImGui::DragFloat(property.name.c_str(), &value)) {
          assets_repository->set_value(asset.id(), name, value);
        }
        break;
      }
      case schema_type::INT64: {
        assert(asset.at(name).is_number_integer());
        int32_t value = asset.at(name);
        if (ImGui::InputInt(property.name.c_str(), &value)) {
          assets_repository->set_value(asset.id(), name, value);
        }
        break;
      }
      case schema_type::BOOL: {
        assert(asset.at(name).is_boolean());
        bool value = asset.at(name);
        if (ImGui::Checkbox(property.name.c_str(), &value)) {
          assets_repository->set_value(asset.id(), name, value);
        }
        break;
      }
      case schema_type::OBJECT: {
        assert(asset.at(name).is_object());
        ImGui::Text("%s", property.name.c_str());
        ImGui::Indent();
        auto& schema = schema_registry->get_schema(property.schema);
        auto& obj = static_cast<const class asset&>(asset.at(name));
        for (auto& prop : schema.properties) {
          data_ui(obj, prop.name, prop, ++id);
        }
        ImGui::Unindent();
        break;
      }
      case schema_type::STRING:break;
      case schema_type::ARRAY: {
        assert(asset.at(name).is_array());
        ImGui::Text("%s", property.name.c_str());
        ImGui::Indent();
//        auto& schema = schema_registry->get_schema(property.schema);
//        auto& arr = static_cast<const asset_array&>(asset.at(name));
//        for (auto& prop : schema.properties) {
//          data_ui(obj, prop.name, prop, ++id);
//        }
        ImGui::Unindent();
      }
      case schema_type::COUNT:
        break;
    }
    ImGui::PopID();
  }

  void gui() override {
    if (!asset_)
      return;

    auto schema = schema_registry->find_schema(asset_type_);
    if (schema == schema_registry->end()) {
      logger::core::Warning("Couldn't find schema for type {}", meta::type(asset_type_).name());
      return;
    }

    ImGui::Text("%s", meta::get_type(asset_type_).name().data());
    ImGui::Indent();

    uint32_t id = 0;
    for (auto& prop : schema->properties) {
      data_ui(*asset_, prop.name, prop, ++id);
    }
    ImGui::Unindent();
  };

  void set_asset(const asset* asset, meta::typeid_t asset_type) {
    asset_ = asset;
    asset_type_ = asset_type;
  }

 private:
  const asset* asset_ = nullptr;
  meta::typeid_t asset_type_;
};

class entity_tree_tab : public editor_tab {
 private:
  static constexpr const char* const DRAG_DROP_TYPE = "DRAG_DROP_NODE";

 public:
  [[nodiscard]] std::string_view name() const override {
    return "Entity Graph";
  }

  void gui() override {
    const ImGuiTreeNodeFlags base_flag =
        ImGuiTreeNodeFlags_None
        | ImGuiTreeNodeFlags_SpanAvailWidth
        | ImGuiTreeNodeFlags_SpanFullWidth
        | ImGuiTreeNodeFlags_AllowItemOverlap
        | ImGuiTreeNodeFlags_OpenOnArrow
        | ImGuiTreeNodeFlags_FramePadding
    ;

    if (!asset_)
      return;

    if (version_ != asset_->version()) {

      version_ = asset_->version();
    }

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 4));
    ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, 8);
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));

    int recursion_depth = 0;
    visit_children(asset_, [&](const asset& asset, uint32_t depth) {
      while (recursion_depth > depth) {
        ImGui::TreePop();
        recursion_depth--;
      }
      assert(recursion_depth == depth);

      std::string id = asset.at("__guid");
      std::string name = "[noname]";
      if (asset.contains("name")) {
        name = asset.at("name").get<std::string>();
      }

      bool leaf = !asset.contains("children") || asset.at("children").get<asset_array&>().empty();
      bool selected = id == selected_id_;
      bool has_components = asset.contains("components");

      ImGuiTreeNodeFlags flag = base_flag;
      if (leaf && !has_components) flag |= ImGuiTreeNodeFlags_Leaf;
      if (leaf && has_components) flag |= ImGuiTreeNodeFlags_Bullet;
      if (selected) flag |= ImGuiTreeNodeFlags_Selected;

      bool open = ImGui::TreeNodeEx(id.c_str(), flag, name.c_str(), "");
      if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
        selected_id_ = id;
      }

      if (ImGui::BeginDragDropSource(
          ImGuiDragDropFlags_AcceptNoPreviewTooltip |
              ImGuiDragDropFlags_SourceNoHoldToOpenOthers |
              ImGuiDragDropFlags_AcceptNoDrawDefaultRect)) {
        ImGui::SetDragDropPayload(DRAG_DROP_TYPE, id.data(), id.size());
        ImGui::Text(name.c_str(), "");
        ImGui::EndDragDropSource();
      }

      if (open && has_components) {
        for (auto& [type_name, comp] : asset.at("components").get<class asset&>()) {

          const class asset& comp_asset = comp;
          std::string comp_id = comp_asset.at("__guid");
          ImGuiTreeNodeFlags comp_flag = base_flag | ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
          if (comp_id == selected_id_) comp_flag |= ImGuiTreeNodeFlags_Selected;

//          ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(120, 240, 120, 255));
          ImGui::TreeNodeEx(comp_id.c_str(), comp_flag, type_name.c_str(), "");
//          ImGui::PopStyleColor();

          if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen()) {
            selected_id_ = comp_id;
            properties->set_asset(&comp_asset, meta::get_type(type_name.c_str()).id());
          }
        }
      }

      if (open) recursion_depth++;
      if (!open || leaf) return visit_children_result::CONTINUE;

      return visit_children_result::RECURSE;
    });

    while (recursion_depth--) {
      ImGui::TreePop();
    }

    ImGui::PopStyleVar(3);
  }

  void load(const asset* asset) {
    asset_ = asset;
    version_ = asset->version();

//    logger::core::Info("[entity_tree_tab] Loaded asset {}", );
  }

//  void change_parent(
//      const guid& guid,
//      const asset::json_pointer& parent_ptr,
//      const asset::json_pointer& new_parent_ptr
//    ) {
//
//    std::string id = guid.str();
//    auto& new_parent = asset_->at(new_parent_ptr);
//    auto& parent = asset_->at(parent_ptr);
//
//    if (parent.contains("children")) {
//      auto& children = parent.at("children");
//      auto it = std::find_if(
//          children.begin(),
//          children.end(),
//          [&](const auto &item) {
//            return item.contains("__guid") && item.at("__guid") == id;
//          }
//      );
//
//      auto node = *it;
//      children.erase(it);
//
//      if (!new_parent.contains("children"))
//        new_parent["children"] = {};
//
//      new_parent.at("children").push_back(std::move(node));
//    }
//  }

 private:
  const asset* asset_ = {};
  std::string selected_id_ = {};
  uint32_t version_ = {};
};

class scene_tab : public editor_tab {
 public:
  [[nodiscard]] std::string_view name() const override {
    static const char* _name = "Scene";
    return _name;
  }

  void gui() override {

    propagate_asset_changes(*world_, *assets_repository);

    ImVec2 scale = ImGui::GetIO().DisplayFramebufferScale;
    ImVec2 size = ImGui::GetContentRegionAvail();
    vec2i resolution {int(size.x * scale.x), int(size.y * scale.y)};

    viewport_.resize(resolution);

    viewer_registry->request_render({
      .world = world_.get(),
      .camera = {
        .view = mat4::trs(camera_transform_.position, camera_transform_.rotation, camera_transform_.scale),
        .projection = mat4::perspective(
          80.0f * math::DEG_TO_RAD,
          (float) resolution.x / (float) resolution.y,
          0.1f,
          100.0f
        )
      },
      .color_target = viewport_.target()
    });

    ImGui::Image((ImTextureID) viewport_.image_id(), size, {0, 1}, {1, 0});
  }

  scene_tab()
    : world_(std::make_unique<world>())
    , camera_transform_(transform::from_matrix(mat4::trs(
          vec3 { 0.0f, -3.0f, 6.0f }, quat::identity(), vec3 { 1.0f, 1.0f, 1.0f }
      )))
    , viewport_(renderer)
  {
    world_->load_from_asset(*assets_repository->get_asset("assets/scenes/start_scene.entity"));
    render_pipeline->on_render_connect(this, &scene_tab::on_render_callback);
  }

  ~scene_tab() override {
    render_pipeline->on_render_disconnect(this, &scene_tab::on_render_callback);
  }

 private:
  void on_render_callback(class renderer& _renderer, render_command_buffer& render_cmd_buf, resource_command_buffer& resource_cmd_buf) {}

 private:
  std::unique_ptr<world> world_;
  transform camera_transform_;
  viewport viewport_;
};

void load_editor_plugin(std::istringstream*, systems_registry& reg) {
  assets_repository = reg.get<class asset_repository>();
  viewer_registry   = reg.get<class viewer_registry>();
  renderer          = reg.get<class renderer>();
  render_pipeline   = reg.get<class render_pipeline>();
  schema_registry   = reg.get<class schema_registry>();


  filebrowser       = reg.add<editor_tab>(std::make_unique<filebrowser_tab>());
  entity_tree       = reg.add<editor_tab>(std::make_unique<entity_tree_tab>());
  properties        = reg.add<editor_tab>(std::make_unique<properties_tab>());
  scene             = reg.add<editor_tab>(std::make_unique<scene_tab>());

  entity_tree->load(assets_repository->get_asset("assets/scenes/start_scene.entity"));
}

void unload_editor_plugin(std::ostringstream*, systems_registry& reg){
  reg.erase<editor_tab>(properties);
  reg.erase<editor_tab>(entity_tree);
  reg.erase<editor_tab>(filebrowser);
  reg.erase<editor_tab>(scene);
}
