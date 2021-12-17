#include "entity_asset_editor.h"
#include "editor_gui.h"
#include "core/assets/assets.h"
#include "editor/gui/gui.h"
#include "editor/gui/imgui_renderer.h"
#include "core/meta/registration.h"
#include "base/event.h"
#include "core/render_texture.h"
#include "core/components/camera_component.h"
#include "core/renderer.h"
#include "core/assets/resources.h"
#include "core/assets/shader.h"

static std::unique_ptr<world> world_;
static assets::handle entity_;
static event<const assets::handle&> on_entity_load;

static void set_entity(assets::repository* p, assets::handle e);

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
  if (first->contains(CHILDREN) && first->at(CHILDREN).is_array()) {
    auto children = first->at(CHILDREN);
    for (auto it = children.begin(); it != children.end(); ++it) {
      if (auto result = visit_children(it, op, depth + 1))
        return result;
    }
  }
  return 0;
}

class entity_graph_gui : public editor_gui {
 private:
  std::string selected_id;

 public:
  void start(assets::repository* p) override {
    // TODO:
    set_entity(p, p->load(fs::absolute("assets/scenes/start_scene.entity")));
  }

  void gui(gui_renderer* gui_renderer) override {
    const ImGuiTreeNodeFlags base_flag = ImGuiTreeNodeFlags_None
        | ImGuiTreeNodeFlags_SpanAvailWidth
        | ImGuiTreeNodeFlags_SpanFullWidth
        | ImGuiTreeNodeFlags_AllowItemOverlap
        | ImGuiTreeNodeFlags_OpenOnArrow
        | ImGuiTreeNodeFlags_FramePadding
        ;

    gui_renderer->set_context();

    if (gui::GetCurrentContext()) {
      gui::Begin("Entity graph");

      if (entity_) {

        int recursion_depth = 0;
        visit_children(entity_, [&](asset& asset, uint32_t depth) {
          while (recursion_depth > depth) {
            gui::TreePop();
            recursion_depth--;
          }
          assert(recursion_depth == depth);

          std::string id = asset.at("__guid");

          bool leaf = !asset.contains("children") || asset.at("children").empty();
          bool selected = id == selected_id;

          ImGuiTreeNodeFlags flag = base_flag;
          if (leaf) flag |= ImGuiTreeNodeFlags_Leaf;
          if (selected) flag |= ImGuiTreeNodeFlags_Selected;

          bool open = ImGui::TreeNodeEx(id.c_str(), flag, id.c_str(), "");
          if (ImGui::IsItemClicked()) {
            selected_id = id;
          }

          if (open) recursion_depth++;
          if (!open || leaf) return visit_children_result::CONTINUE;

          return visit_children_result::RECURSE;
        });

        while (recursion_depth--) {
          gui::TreePop();
        }
      }

      gui::End();
    }
  }
};

void set_entity(assets::repository* p, assets::handle e) {
  entity_ = std::move(e);

  // TODO: world->clear();
  {
    for (auto it = world_->root(); it.is_valid();) {
      auto next_it = it = world_->next(it);
      world_->destroy_entity(it);
      it = next_it;
    }
  }

  world_->load_from_asset(p, *entity_);

  on_entity_load(entity_);
}

class scene_view_gui : public editor_gui {

 public:
  void start(assets::repository*) override;
  void gui(gui_renderer* gui_renderer) override;

 private:
  void move_camera(world* w, const vec2 &delta);
  void zoom_camera(world* w, float delta);
  void rotate_camera(world* w, const vec2 &delta);

 private:
  resources::handle<shader> grid_shader_;
  resources::handle<shader> picking_shader_;
  gfx::uniform_handle picking_id_uniform_;
  entity camera_;
  std::unique_ptr<render_texture> render_texture_;
  std::unique_ptr<render_texture> picking_render_texture_;
};

void scene_view_gui::gui(gui_renderer *gui_renderer) {
  gui_renderer->set_context();

  gui::Begin("Scene View");

  ImVec2 scale = gui::GetIO().DisplayFramebufferScale;
  ImVec2 size = gui::GetContentRegionAvail();
  vec2i resolution {int(size.x * scale.x), int(size.y * scale.y)};

  if (!render_texture_ ||
      resolution.x != render_texture_->texture().width() ||
      resolution.y != render_texture_->texture().height()) {

    render_texture_.reset();
    render_texture_ = std::make_unique<render_texture>(resolution.x, resolution.y, gfx::texture_format::RGBA8, gfx::texture_wrap{}, gfx::texture_filter{}, gfx::texture_flags::None);
  }

  if (!picking_render_texture_ ||
      resolution.x != picking_render_texture_->texture().width() ||
      resolution.y != picking_render_texture_->texture().height()) {

    picking_render_texture_.reset();
    picking_render_texture_ = std::make_unique<render_texture>(resolution.x, resolution.y, gfx::texture_format::RGBA8, gfx::texture_wrap{}, gfx::texture_filter{}, gfx::texture_flags::None);
  }

  auto camera_predicate = [] (entity e, const camera_component& c) { return (c.tag & camera_component::tag_t::Editor) == camera_component::tag_t::Editor; };
  renderer::update_views(world_.get(), {0,0, (float) resolution.x, (float) resolution.y}, render_texture_->handle(), camera_predicate);
  renderer::render(world_.get(), camera_predicate);

  // render grid
  {
    static float size = 100.0f;
    static float hs = size * 0.5f;

    static const float uv_max = 0.5f;
    static const float uv_min = -0.5f;
    static float vertices[] = {
        // pos          // tex coords
        -hs, -hs, 0.0f, uv_min, uv_min,
        hs, -hs, 0.0f, uv_max, uv_min,
        hs,  hs, 0.0f, uv_max, uv_max,
        hs,  hs, 0.0f, uv_max, uv_max,
        -hs,  hs, 0.0f, uv_min, uv_max,
        -hs, -hs, 0.0f, uv_min, uv_min,
    };

    static gfx::vertexbuf_handle vb = gfx::create_vertex_buffer(
        gfx::make_ref(vertices, sizeof(vertices)),
        6,
        {
            {gfx::attribute::binding::Position, gfx::attribute::format::Float3()},
            {gfx::attribute::binding::TexCoord0, gfx::attribute::format::Float2()}
        });

    renderer::render(mat4::look_at(vec3::zero(), vec3::up(), vec3::forward()), grid_shader_->handle(), vb, gfx::indexbuf_handle::invalid(), world_->component<camera_component>(camera_));
  }

  gui::Image((ImTextureID)(intptr_t)render_texture_->texture().handle().id, size, {0,1}, {1, 0});

//  if (gui::IsMouseClicked(ImGuiMouseButton_Left)) {
//    uint64_t id =
//    gfx::set_uniform(picking_id_uniform_, )
//    renderer::update_views(world_.get(), {0,0, (float) resolution.x, (float) resolution.y}, picking_render_texture_->handle(), camera_predicate);
//    renderer::render(world_.get(), camera_predicate, picking_shader_);
//  }

  ImGuiIO& io = gui::GetIO();
  if (gui::IsWindowHovered() && io.MouseWheel != 0.0f) {
    zoom_camera(world_.get(), io.MouseWheel);
  }

  if (gui::IsWindowFocused()) {
    vec2 move_delta {};
    vec2 rotate_delta {};

    bool moving = gui::IsKeyDown(key::LeftAlt) && gui::IsKeyDown(key::LeftSuper);
    bool rotating = !moving && gui::IsKeyDown(key::LeftAlt);
    if (moving) {
      auto mouse_delta = gui::GetMouseDragDelta(ImGuiMouseButton_Left);
      gui::ResetMouseDragDelta(ImGuiMouseButton_Left);
      move_delta.x = -0.1f * mouse_delta.x;
      move_delta.y = 0.1f * mouse_delta.y;
    }
    else if (rotating) {
      auto mouse_delta = gui::GetMouseDragDelta(ImGuiMouseButton_Left);
      gui::ResetMouseDragDelta(ImGuiMouseButton_Left);
      rotate_delta.x = mouse_delta.x;
      rotate_delta.y = mouse_delta.y;
    }
    else {
      if (gui::IsKeyDown(key::Up)) move_delta.y += 1;
      if (gui::IsKeyDown(key::Down)) move_delta.y -= 1;
      if (gui::IsKeyDown(key::Right)) move_delta.x += 1;
      if (gui::IsKeyDown(key::Left)) move_delta.x -= 1;
    }

    if (move_delta != vec2::zero())
      move_camera(world_.get(), move_delta);

    if (rotate_delta != vec2::zero())
      rotate_camera(world_.get(), rotate_delta);
  }

  gui::End();

}

void scene_view_gui::move_camera(world *w, const vec2 &delta) {
  static const float speed = 0.2f;
  transform camera_local = w->local_transform(camera_);
  vec3 direction = camera_local.transform_direction(speed * vec3{delta.x, delta.y});
  w->set_local_position(camera_, camera_local.position + direction);
}

void scene_view_gui::zoom_camera(world *w, float delta) {
  static const float speed = 1.0f;
  transform camera_local = w->local_transform(camera_);
  vec3 direction = camera_local.transform_direction((speed * delta * vec3::forward()));
  w->set_local_position(camera_, camera_local.position + direction);
}

void scene_view_gui::rotate_camera(world *w, const vec2 &delta) {
  static const float distance = 2.0f;
  static const float speed = 0.1f;
  transform camera_local = w->local_transform(camera_);

  const vec3 up = camera_local.up();
  const vec3 right = camera_local.right();
  const vec3 fwd = camera_local.forward();

  vec3 dir = distance * fwd;
  quat rot = quat::axis(vec3::up(), speed * delta.x * math::DEG_TO_RAD);
  rot *= quat::axis(right, speed * delta.y * math::DEG_TO_RAD);

  vec3 center = camera_local.position + dir;
  vec3 local_pos = center + rot * -dir;

  w->set_local_position(camera_, local_pos);
  w->set_local_rotation(camera_, quat::look_at(center - local_pos, rot * up));
}

void scene_view_gui::start(assets::repository* repository) {
  // create camera
  camera_ = world_->create_entity();
  world_->set_local_rotation(camera_, quat::axis(vec3::right(), 30 * math::DEG_TO_RAD));
  world_->set_local_position(camera_, vec3 { 0, 0, 0} );
  auto& camera = world_->add_component<camera_component>(camera_);
  camera.clear_color = { 0.1f, 0.1f, 0.1f, 1.0f };
  camera.tag |= camera_component::tag_t::Editor;
  camera.far = 10'000.0f;

  grid_shader_ = resources::load<shader>(fs::absolute("assets/shaders/EditorGrid.shader"), repository);
  picking_shader_ = resources::load<shader>(fs::absolute("assets/shaders/picking.shader"), repository);

  picking_id_uniform_ = gfx::create_uniform("pick_id");
}

void load_entity_asset_editor(struct engine_events*) {
  register_type(entity_graph_gui);
  register_type(scene_view_gui);

  editor::g_editor_gui->add_editor<entity_graph_gui>();
  editor::g_editor_gui->add_editor<scene_view_gui>();

  world_ = world::create();
}

void unload_entity_asset_editor(struct engine_events*) {
  editor::g_editor_gui->remove_editor<entity_graph_gui>();
  editor::g_editor_gui->remove_editor<scene_view_gui>();

  world_.reset();
}