#include "game_view_gui.h"
#include "core/render_texture.h"
#include "core/world.h"

#include "gfx/gfx.h"
#include "editor/editor_gui.h"
#include "editor/gui/gui.h"
#include "editor/gui/imgui_renderer.h"
#include "core/renderer.h"
#include "core/meta/registration.h"
#include "core/engine_events.h"

class game_view_gui : public editor_gui {
 public:

  void start(assets::repository*) override {}

  void gui(gui_renderer* gui_renderer) override {

    gui_renderer->set_context();

    if (gui::GetCurrentContext()) {
      gui::Begin("Game View");

      ImVec2 scale = gui::GetIO().DisplayFramebufferScale;
      ImVec2 size = gui::GetContentRegionAvail();
      vec2i resolution {int(size.x * scale.x), int(size.y * scale.y)};

      if (!render_texture_ ||
          resolution.x != render_texture_->texture().width() ||
          resolution.y != render_texture_->texture().height()) {

        render_texture_.reset();
        render_texture_ = std::make_unique<render_texture>(resolution.x, resolution.y, gfx::texture_format::RGBA8, gfx::texture_wrap{}, gfx::texture_filter{}, gfx::texture_flags::None);
      }

      auto camera_predicate = [] (entity e, const camera_component& c) { return (c.tag & camera_component::tag_t::Game) == camera_component::tag_t::Game; };
      renderer::update_views(ecs::world, {0,0, (float) resolution.x, (float) resolution.y}, render_texture_->handle(), camera_predicate);
      renderer::render(ecs::world, camera_predicate);

      gui::Image((ImTextureID)(intptr_t)render_texture_->texture().handle().id, size, {0,1}, {1, 0});

      gui::End();
    }
  }

 private:
  std::unique_ptr<render_texture> render_texture_;

};

void register_editor(editor_gui_plugin* e) {

}

using event_key_t = event<editor_gui_plugin>::key_t;
static inline event_key_t event_key;

void load_game_view_gui(engine_events* events) {
  register_type(game_view_gui);

  event_key = events->get<editor_gui_plugin*>("register_editor").connect(register_editor);

  editor::g_editor_gui->add_editor<game_view_gui>();
}

void unload_game_view_gui(engine_events* events) {
  editor::g_editor_gui->remove_editor<game_view_gui>();

  events->get<editor_gui_plugin*>("register_editor").disconnect(event_key);
}