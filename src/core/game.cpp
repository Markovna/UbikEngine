#include "core/engine.h"
#include "core/input_system.h"
#include "core/world.h"
#include "core/components/mesh_component.h"
#include "core/components/camera_component.h"
#include "core/plugins_registry.h"
#include "core/renderer.h"
#include "gfx/gfx.h"
#include "platform/window.h"
#include "base/window_event.h"
#include "core/assets/assets.h"
#include "core/meta/registration.h"
#include "core/meta/schema.h"

extern void load_plugins(engine*);

int main(int argv, char* argc[]) {

  char schema_dir[256];
  std::strcpy(schema_dir, std::filesystem::current_path().c_str());
  std::strcat(schema_dir, "/schema");

  meta::load_schemas(schema_dir);

  register_type(link_component);
  register_type(transform_component);
  register_type(camera_component);
  register_type(mesh_component);

  assets::init(std::filesystem::current_path().c_str());

  window window({512, 512});
  gfx::init({.window_handle = window.get_handle(), .resolution = window.get_resolution()});

  engine engine;
  engine.world = new world;
  engine.plugins = new plugins_registry;
  engine.input = new input_system;

  load_plugins(&engine);

  engine.start();
  vec4 viewport;

  bool running = true;
  while (running) {

    window.update();

    window_event event;
    while (window.poll_event(event)) {
      if (event.type == event_type::Close) {
        running = false;
      }
      engine.input->push_event(event);
    }

    engine.update();

    vec2i resolution = window.get_resolution();
    viewport.z = (float) resolution.x;
    viewport.w = (float) resolution.y;
    renderer::render(engine.world, viewport, gfx::framebuf_handle::invalid(), camera_component::kind_t::Game);

    gfx::frame();
  }

  engine.stop();
  delete engine.input;
  delete engine.plugins;
  delete engine.world;

  gfx::shutdown();

  assets::shutdown();
}