#include "core/input_system.h"
#include "core/world.h"
#include "core/plugins.h"
#include "core/meta/registration.h"
#include "core/meta/schema.h"
#include "core/application.h"
#include "gfx/gfx.h"
#include "platform/window.h"
#include "platform/file_system.h"

#include "core/components/mesh_component.h"
#include "core/components/camera_component.h"

#include "editor/gui/gui.h"
#include "editor/gui/imgui_renderer.h"
#include "editor_gui.h"
#include "core/assets/assets.h"
#include "core/assets/resources.h"
#include "core/assets/resource_compiler.h"
#include "core/assets/filesystem_provider.h"
#include "core/serialization.h"

#include "library_loader.h"

#include <vector>

int main(int argc, char* argv[]) {
  const char* plugin_names [] = {
      "sandbox",
      "spin_plugin",
      "sandbox_plugin",
      "game_view_gui",
      "file_browser_gui",
      "entity_asset_editor"
  };

  fs::paths::project(argv[1]);
  logger::init(fs::append(fs::paths::cache(), "log").c_str());

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

  register_component_i(link_component);
  register_component_i(transform_component);
  register_component_i(camera_component);
  register_component_i(mesh_component);

  register_serializer_i(link_component);
  register_serializer_i(transform_component);
  register_serializer_i(camera_component);
  register_serializer_i(mesh_component);

  window window({1024, 512});
  gfx::init({.window_handle = window.get_handle(), .resolution = window.get_resolution()});

  ecs::init_world();
  init_input_system();
  init_plugins_registry();

  init_filesystem_provider();

  assets::init();

  resources::init_compiler();
  resources::compile_all_assets(fs::paths::project().c_str());

  resources::init();

  std::unique_ptr<gui_renderer> gui_renderer = gui_renderer::create(&window, g_fsprovider);

  connect_gui_events(gui_renderer.get(), input);

  editor::init_editor_gui();

  library_loader libs(fs::append(fs::paths::cache(), "libs_tmp").c_str());
  for (auto plugin_name : plugin_names) {
    libs.load(plugin_name, os::find_lib(fs::append(fs::paths::cache(), "libs").c_str(), plugin_name), plugins_reg);
  }

  if (g_application)
    g_application->start(g_fsprovider);

  editor::g_editor_gui->start(g_fsprovider);

  bool running = true;
  while (running) {
    libs.check_hot_reload(plugins_reg);

    window.update();

    window_event event;
    while (window.poll_event(event)) {
      if (event.type == event_type::Close) {
        running = false;
      }
      input->push_event(event);
    }

    gfx::resolution(window.get_resolution());

    gui_renderer->begin_frame();
    gui::begin_dockspace();

    if (g_application)
      g_application->tick();

    ecs::world->update_systems();

    editor::g_editor_gui->gui(gui_renderer.get());

    window.set_cursor(gui_renderer->cursor());
    gui_renderer->end_frame();

    gfx::frame();
  }

  if (g_application)
    g_application->stop();

  for (auto plugin_name : plugin_names) {
    libs.unload(plugin_name, plugins_reg);
  }

  editor::shutdown_editor_gui();

  disconnect_gui_events(gui_renderer.get(), input);

  shutdown_filesystem_provider();

  shutdown_input_system();
  shutdown_plugins_registry();
  ecs::shutdown_world();

  resources::shutdown();
  resources::shutdown_compiler();

  assets::shutdown();

  meta::shutdown();

  gfx::shutdown();
  return 0;
}

