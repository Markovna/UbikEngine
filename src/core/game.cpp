
#include <core/assets/assets.h>
#include "core/input_system.h"
#include "core/world.h"
#include "core/components/mesh_component.h"
#include "core/components/camera_component.h"
#include "core/renderer.h"
#include "gfx/gfx.h"
#include "platform/window.h"
#include "base/window_event.h"
#include "core/meta/registration.h"
#include "core/meta/schema.h"
#include "core/plugins.h"
//#include "core/plugins_registry.h"

extern void load_plugins(plugins*);

int main(int argc, char* argv[]) {

  fs::paths::project(fs::current_path().c_str());
  logger::init(fs::absolute("log").c_str());

  meta::init();
  meta::load_schemas(fs::absolute("schema").c_str());

  register_type(color);
  register_type(transform);
  register_type(vec2);
  register_type(vec3);
  register_type(vec4);
  register_type(vec2i);
  register_type(vec3i);
  register_type(vec4i);
  register_type(quat);

  register_type(link_component);
  register_type(transform_component);
  register_type(camera_component);
  register_type(mesh_component);

  assets::init();

  window window({512, 512});
  gfx::init({.window_handle = window.get_handle(), .resolution = window.get_resolution()});

  ecs::init_world();
  init_input_system();
  init_plugins_registry();
  load_plugins(plugins_reg);

  assets::filesystem_provider* provider = new assets::filesystem_provider;
  provider->add(fs::paths::project());

  assets::init(provider);

  ecs::world->start_systems();
  vec4 viewport {};

  bool running = true;
  while (running) {

    window.update();

    window_event event;
    while (window.poll_event(event)) {
      if (event.type == event_type::Close) {
        running = false;
      }
      input->push_event(event);
    }

    gfx::resolution(window.get_resolution());

    ecs::world->update_systems();

    vec2i resolution = window.get_resolution();
    viewport.z = (float) resolution.x, viewport.w = (float) resolution.y;
    renderer::update_views(ecs::world, viewport, gfx::framebuf_handle::invalid());
    renderer::render(ecs::world);

    gfx::frame();
  }

  ecs::world->stop_systems();

  shutdown_input_system();
  shutdown_plugins_registry();
  ecs::shutdown_world();

  assets::shutdown();

  meta::shutdown();

  gfx::shutdown();
  return 0;
}