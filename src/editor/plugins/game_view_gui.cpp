#include <core/render_texture.h>
#include "game_view_gui.h"

#include "gfx/gfx.h"
#include "core/engine.h"
#include "editor/editor_gui_i.h"
#include "editor/gui/gui.h"
#include "editor/gui/imgui_renderer.h"
#include "core/plugins_registry.h"
#include "core/renderer.h"

class game_view_gui : public plugin<editor_gui_i> {
 public:

  void gui(engine* e, gui_renderer* gui_renderer) override {

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

      renderer::render(e->world, {0,0, (float) resolution.x, (float) resolution.y}, render_texture_->handle(), camera_component::kind_t::Game);

      gui::Image((ImTextureID)(intptr_t)render_texture_->texture().handle().id, size, {0,1}, {1, 0});

      gui::End();
    }
  }

 private:
  std::unique_ptr<render_texture> render_texture_;

};

extern "C" void load_game_view_gui(engine* e) {
  e->plugins->add<game_view_gui>("game_view_gui");
}

extern "C" void unload_game_view_gui(engine* e) {
  e->plugins->remove("game_view_gui");

}