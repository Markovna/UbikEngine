#include "scene_view_gui.h"

#include "base/color.h"

#include "core/engine.h"
#include "core/world.h"
#include "core/plugins_registry.h"
#include "core/render_texture.h"
#include "core/renderer.h"
#include "core/components/camera_component.h"

#include "editor/editor_gui_i.h"
#include "editor/gui/imgui_renderer.h"

class scene_view_gui : public plugin<editor_gui_i> {
 public:
  void gui(engine* e, gui_renderer* gui_renderer) override;

 private:
  void move_camera(world* w, const vec2 &delta);
  void zoom_camera(world* w, float delta);
  void rotate_camera(world* w, const vec2 &delta);

 private:
  entity camera_;
  std::unique_ptr<render_texture> render_texture_;
};

void scene_view_gui::gui(engine *e, gui_renderer *gui_renderer) {
  gui_renderer->set_context();

  if (!e->world->valid(camera_)) {
    // create camera
    camera_ = e->world->create_entity();
    e->world->set_local_rotation(camera_, quat::axis(vec3::right(), 30 * math::DEG_TO_RAD));
    e->world->add_component<camera_component>(camera_, camera_component::kind_t::Editor)
        .clear_color = { 0.1f, 0.1f, 0.1f, 1.0f };

  }

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

  renderer::render(e->world, {0,0, (float) resolution.x, (float) resolution.y}, render_texture_->handle(), camera_component::kind_t::Editor);

  gui::Image((ImTextureID)(intptr_t)render_texture_->texture().handle().id, size, {0,1}, {1, 0});

  ImGuiIO& io = gui::GetIO();
  if (gui::IsWindowHovered() && io.MouseWheel != 0.0f) {
    zoom_camera(e->world, io.MouseWheel);
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
      move_camera(e->world, move_delta);

    if (rotate_delta != vec2::zero())
      rotate_camera(e->world, rotate_delta);
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

void load_scene_view_gui(engine* e) {
  e->plugins->add<scene_view_gui>("scene_view_gui");
}

void unload_scene_view_gui(engine* e) {
  e->plugins->remove("scene_view_gui");
}