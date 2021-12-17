
#include <core/assets/assets.h>
#include "core/input_system.h"
#include "core/world.h"

#include "gfx/gfx.h"
#include "platform/window.h"
#include "base/window_event.h"
#include "core/meta/registration.h"
#include "core/meta/schema.h"
#include "core/assets/filesystem_provider.h"
#include "core/application.h"
#include "engine_events.h"

extern void load_plugins(engine_events*);

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

  window window({512, 512});
  gfx::init({.window_handle = window.get_handle(), .resolution = window.get_resolution()});

  engine_events events;

  ecs::init_world();
  init_input_system();
  load_plugins(&events);

  init_filesystem_provider();

  assets::init();

  resources::init();

  if (g_application)
    g_application->start(g_fsprovider);

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

    if (g_application)
      g_application->tick();

    ecs::world->update_systems();

    vec2i resolution = window.get_resolution();
    viewport.z = (float) resolution.x, viewport.w = (float) resolution.y;
    renderer::update_views(ecs::world, viewport, gfx::framebuf_handle::invalid());
    renderer::render(ecs::world);

    gfx::frame();
  }

  ecs::world->stop_systems();

  if (g_application)
    g_application->stop();

  shutdown_filesystem_provider();

  shutdown_input_system();
  ecs::shutdown_world();

  resources::shutdown();
  assets::shutdown();
  meta::shutdown();
  gfx::shutdown();

  return 0;
}