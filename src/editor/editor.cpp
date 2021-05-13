#include "core/engine.h"
#include "core/renderer.h"
#include "core/input_system.h"
#include "core/world.h"
#include "core/plugins_registry.h"
#include "gfx/gfx.h"
#include "platform/window.h"
#include "library_registry.h"

#include <filesystem>
#include <vector>

int main(int argc, char* argv[]) {
  const std::vector<std::string> plugin_names = {
      "spin_plugin"
  };

  std::filesystem::path project_path(argv[1]);
  std::filesystem::path libs_folder(project_path);
  libs_folder.append("libs");

  std::filesystem::path temp_folder(project_path);
  temp_folder.append("temp");

  library_registry libs(libs_folder.c_str(), temp_folder.c_str());

  window window({512, 512});
  gfx::init({.window_handle = window.get_handle(), .resolution = window.get_resolution()});

  engine engine;
  engine.world = new world;
  engine.plugins = new plugins_registry;
  engine.input = new input_system;

  for (auto& plugin_name : plugin_names) {
    libs.load(plugin_name.c_str(), engine.plugins);
  }

  engine.start();

  bool running = true;
  while (running) {

    libs.check_reload(engine.plugins);

    window.update();

    window_event event;
    while (window.poll_event(event)) {
      if (event.type == event_type::Close) {
        running = false;
      }
      engine.input->push_event(event);
    }

    engine.update();

    renderer::render(engine.world);

    gfx::frame();
  }

  libs.unload_all(engine.plugins);

  engine.stop();
  delete engine.input;
  delete engine.plugins;
  delete engine.world;

  gfx::shutdown();
  return 0;
}

