#include "core/engine.h"
#include "core/renderer.h"
#include "core/input_system.h"
#include "core/world.h"
#include "core/plugins_registry.h"
#include "core/assets/assets.h"
#include "core/meta/registration.h"
#include "core/meta/schema.h"
#include "core/serialization.h"
#include "gfx/gfx.h"
#include "platform/window.h"
#include "library_registry.h"

#include "core/components/mesh_component.h"
#include "core/components/camera_component.h"

#include <filesystem>
#include <vector>

int main(int argc, char* argv[]) {
  const std::vector<std::string> plugin_names = {
      "spin_plugin", "sandbox_plugin"
  };

  std::filesystem::path project_path(argv[1]);
  std::filesystem::path libs_folder(project_path);
  libs_folder.append("libs");

  std::filesystem::path temp_folder(project_path);
  temp_folder.append("temp");

  library_registry libs(libs_folder.c_str(), temp_folder.c_str());

  char schema_dir[256];
  std::strcpy(schema_dir, argv[1]);
  std::strcat(schema_dir, "/schema");

  meta::load_schemas(schema_dir);

  register_type(link_component);
  register_type(transform_component);
  register_type(camera_component);
  register_type(mesh_component);

  assets::init(argv[1]);

  window window({512, 512});
  gfx::init({.window_handle = window.get_handle(), .resolution = window.get_resolution()});

  vec4 viewport {};

  engine engine {};
  engine.world = new world;
  engine.plugins = new plugins_registry;
  engine.input = new input_system;

  for (auto& plugin_name : plugin_names) {
    libs.load(plugin_name.c_str(), &engine);
  }

  engine.start();

  ::asset data;
  engine.world->save_entity_to_asset(data, engine.world->root());
  std::cout << data.dump(2) << "\n";

  bool running = true;
  while (running) {

    libs.check_reload(&engine);

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

  libs.unload_all(&engine);

  engine.stop();
  delete engine.input;
  delete engine.plugins;
  delete engine.world;

  gfx::shutdown();

  assets::shutdown();

  return 0;
}

