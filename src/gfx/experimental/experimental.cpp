#include <platform/window.h>
#include <gfx/experimental/gfx.h>
#include <gfx/experimental/renderer.h>
#include <gfx/experimental/render_context_opengl.h>
#include "asset_repository.h"
#include "shader_compiler_opengl.h"
#include "shader_repository.h"
#include "texture_compiler.h"
#include "systems_registry.h"
#include "mesh_component.h"
#include "render_pipeline.h"
#include "gui.h"
#include "imgui.h"
#include "render_target_buffer.h"
#include "base/timer.h"

#include <core/engine_events.h>
#include <editor/library_loader.h>
#include <core/application.h>
#include <core/input_system.h>
#include <core/simulation_events.h>
#include <core/simulation.h>

int main(int argc, char* argv[]) {

  logger::init(fs::current_path().append("log").c_str());

  systems_registry registry;

  auto assets_repository = registry.set<::assets_repository>(std::make_unique<::assets_repository>());
  auto assets_filesystem = registry.set<::assets_filesystem>(std::make_unique<::assets_filesystem>());
  auto renderer = registry.set<::renderer>(std::make_unique<::renderer>(render_context_opengl::create));

  auto shader_compiler = shader_compiler_opengl::create();
  auto shader_repository = registry.set<::shader_repository>(std::make_unique<::shader_repository>(shader_compiler.get()));
  auto render_pipeline = registry.set<::render_pipeline>(std::make_unique<::render_pipeline>());
  auto input_system = registry.set<::input_system>(std::make_unique<::input_system>());

  assets_filesystem->load_assets(*assets_repository, { ".entity", ".meta", ".shader" });

  compile_shaders(*assets_repository, *assets_filesystem);
  compile_textures({".jpg", ".png"}, *assets_repository, *assets_filesystem);

  window window({512, 512});

  swap_chain swap_chain = renderer->create_swap_chain(window.get_handle());

  resource_command_buffer* resource_command_buffer = renderer->create_resource_command_buffer();
  shader_repository->compile(*assets_repository->load("assets/shaders/TestShader.shader"), assets_repository.get(), resource_command_buffer);
  shader_repository->compile(*assets_repository->load("assets/shaders/GUIShader.shader"), assets_repository.get(), resource_command_buffer);
  shader_repository->compile(*assets_repository->load("assets/shaders/EditorGrid.shader"), assets_repository.get(), resource_command_buffer);
  renderer->submit(resource_command_buffer);

  load_mesh(registry);
  load_gui(registry);

  fs::path libs_folder = fs::append(fs::current_path(), ".ubik/libs");
  library_loader libs;
  libs.load("sandbox", os::find_lib(libs_folder.c_str(), "sandbox"), registry);

  auto app = registry.get<application>();
  app->start();

  world world;

  gui gui_renderer { &window };

  connect_gui_events(gui_renderer, *input_system);
  uniform_handle texture_uniform;
  render_target render_target;

  auto simulation_view = registry.view<simulation>();

  for (auto& sim : simulation_view) {
    sim->start(world);
  }

  timer timer;
  timer.restart();

  bool running = true;
  while (running) {

    libs.check_hot_reload(registry);

    window.update();

    float dt = timer.restart().as_milliseconds();
    std::cout << "delta time: " << dt << "\n";

    window_event event;
    while (window.poll_event(event)) {
      if (event.type == event_type::Close) {
        running = false;
      } else if (event.type == event_type::Resize) {
        renderer->resize_swap_chain(swap_chain, event.resize.resolution);
      }
      input_system->push_event(event);
    }

    app->tick();

    for (auto& sim : simulation_view) {
      std::cout << "invoke sim\n";
      sim->update(world, dt);
    }

    renderer->begin_frame();

    gui_renderer.begin();

    gui_begin_dockspace();

    {
      if (!texture_uniform) {
        auto* res_cmd_buf = renderer->create_resource_command_buffer();

        texture_uniform = res_cmd_buf->create_uniform({
            {.type = uniform_type::SAMPLER, .binding = 0}
        });

        render_target = create_render_target(*res_cmd_buf, window.get_resolution());
        res_cmd_buf->set_uniform(texture_uniform, 0, render_target.color_target);

        renderer->submit(res_cmd_buf);
      }

      auto camera_view = world.view<transform_component, camera_component>();
      for (auto e : camera_view) {
        vec2i size = window.get_resolution();
        viewer viewer {
            .world = &world,
            .camera = &camera_view.get<camera_component>(e),
            .transform = &camera_view.get<transform_component>(e),
            .color_target = render_target.fb_handle,
            .size = {(float) size.x, (float) size.y}
        };

        render_pipeline->render(viewer, *renderer);
      }

      ImGui::Begin("Hello, World");
      ImGui::Image((ImTextureID)(intptr_t) texture_uniform.id, { 200, 200 }, {0, 1}, {1, 0});
      ImGui::End();
    }

    window.set_cursor(gui_renderer.cursor());

    gui_renderer.end(swap_chain.back_buffer_handle);

    renderer->swap(swap_chain);
  }

  for (auto& sim : simulation_view) {
    sim->stop();
  }

  disconnect_gui_events(gui_renderer, *input_system);

  app->stop();

}
