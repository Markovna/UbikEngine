#include "core/engine.h"
#include "core/renderer.h"
#include "core/input_system.h"
#include "core/world.h"
#include "core/plugins_registry.h"
#include "core/assets/asset_handle.h"
#include "core/meta/registration.h"
#include "core/meta/schema.h"
#include "core/serialization.h"
#include "gfx/gfx.h"
#include "platform/window.h"
#include "platform/file_system.h"
#include "core/assets/shader.h"

#include "core/components/mesh_component.h"
#include "core/components/camera_component.h"

#include "editor/gui/gui.h"
#include "editor/gui/imgui_renderer.h"
#include "editor/tools/asset_compiler.h"
#include "editor_gui_i.h"

#include "library_loader.h"

#include <vector>

int main(int argc, char* argv[]) {
  const std::vector<std::string> plugin_names = {
      "spin_plugin", "sandbox_plugin", "game_view_gui"
  };

  fs::paths::project(argv[1]);
  logger::init(fs::append(fs::paths::cache(), "log").c_str());

  assets::compile_assets(fs::paths::project().c_str());

  library_loader libs(fs::append(fs::paths::cache(), "libs_tmp").c_str());

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

  window window({1024, 512});
  gfx::init({.window_handle = window.get_handle(), .resolution = window.get_resolution()});

  std::unique_ptr<gui_renderer> gui_renderer = gui_renderer::create(&window);

  engine engine {};
  engine.world = new world;
  engine.plugins = new plugins_registry;
  engine.input = new input_system;

  engine.input->on_resize.connect(gui_renderer.get(), &gui_renderer::on_resize);
  engine.input->on_key_press.connect(gui_renderer.get(), &gui_renderer::on_key_pressed);
  engine.input->on_key_release.connect(gui_renderer.get(), &gui_renderer::on_key_released);
  engine.input->on_mouse_down.connect(gui_renderer.get(), &gui_renderer::on_mouse_down);
  engine.input->on_mouse_move.connect(gui_renderer.get(), &gui_renderer::on_mouse_move);
  engine.input->on_mouse_up.connect(gui_renderer.get(), &gui_renderer::on_mouse_up);
  engine.input->on_scroll.connect(gui_renderer.get(), &gui_renderer::on_scroll);
  engine.input->on_text.connect(gui_renderer.get(), &gui_renderer::on_text_input);

  for (auto& plugin_name : plugin_names) {
    libs.load(plugin_name.c_str(), os::find_lib(fs::append(fs::paths::cache(), "libs").c_str(), plugin_name.c_str()), &engine);
  }

  engine.start();

  bool running = true;
  while (running) {

    libs.update(&engine);

    window.update();

    window_event event;
    while (window.poll_event(event)) {
      if (event.type == event_type::Close) {
        running = false;
      }
      engine.input->push_event(event);
    }

    gui_renderer->begin_frame();
    gui::begin_dockspace();

    engine.update();

    for (editor_gui_i& plugin : engine.plugins->view<editor_gui_i>()) {
      plugin.gui(&engine, gui_renderer.get());
    }

    window.set_cursor(gui_renderer->cursor());
    gui_renderer->end_frame();

    gfx::frame();
  }

  for (auto& plugin_name : plugin_names) {
    libs.unload(plugin_name.c_str(), &engine);
  }

  engine.input->on_resize.disconnect(gui_renderer.get(), &gui_renderer::on_resize);
  engine.input->on_key_press.disconnect(gui_renderer.get(), &gui_renderer::on_key_pressed);
  engine.input->on_key_release.disconnect(gui_renderer.get(), &gui_renderer::on_key_released);
  engine.input->on_mouse_down.disconnect(gui_renderer.get(), &gui_renderer::on_mouse_down);
  engine.input->on_mouse_move.disconnect(gui_renderer.get(), &gui_renderer::on_mouse_move);
  engine.input->on_mouse_up.disconnect(gui_renderer.get(), &gui_renderer::on_mouse_up);
  engine.input->on_scroll.disconnect(gui_renderer.get(), &gui_renderer::on_scroll);
  engine.input->on_text.disconnect(gui_renderer.get(), &gui_renderer::on_text_input);

  engine.stop();
  delete engine.input;
  delete engine.plugins;
  delete engine.world;

  gfx::shutdown();
  return 0;
}

