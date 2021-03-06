#include <platform/window.h>
#include <gfx/gfx.h>
#include <core/renderer.h>
#include <gfx/render_context_opengl.h>
#include "core/assets_filesystem.h"
#include "gfx/shader_compiler_opengl.h"
#include "gfx/shader_repository.h"
#include "core/texture_compiler.h"
#include "core/systems_registry.h"
#include "core/register_components.h"
#include "core/render_pipeline.h"
#include "gui.h"
#include "imgui.h"
#include "base/timer.h"
#include "core/viewer_registry.h"
#include "core/components/mesh_component.h"

#include <core/engine_events.h>
#include <editor/library_loader.h>
#include <core/application.h>
#include <core/input_system.h>
#include <core/simulation_events.h>
#include <core/simulation.h>
#include <editor/editor_tab.h>
#include <core/meta/interface_registry.h>
#include <core/schema.h>
#include <base/color.h>
#include <core/dcc_asset.h>
#include <editor/editor_tab_manager.h>
#include "core/asset_repository.h"

int main(int argc, char* argv[]) {
  fs::project_path(argv[1]);

  logger::init(fs::project_path().append("log").c_str());

  systems_registry registry;

  auto interface_registry = registry.set<::interface_registry>(std::make_unique<::interface_registry>());
  auto assets_repository = registry.set<::asset_repository>(std::make_unique<::asset_repository>());
  auto assets_filesystem = registry.set<::assets_filesystem>(std::make_unique<::assets_filesystem>());
  auto renderer = registry.set<::renderer>(std::make_unique<::renderer>(render_context_opengl::create));

  auto shader_compiler = shader_compiler_opengl::create();
  auto shader_repository = registry.set<::shader_repository>(std::make_unique<::shader_repository>(shader_compiler.get()));
  auto render_pipeline = registry.set<::render_pipeline>(std::make_unique<::render_pipeline>());
  auto input_system = registry.set<::input_system>(std::make_unique<::input_system>());
  auto viewer_registry = registry.set<::viewer_registry>(std::make_unique<::viewer_registry>());
  auto schema_registry = registry.set<::schema_registry>(std::make_unique<::schema_registry>());
  auto gui_renderer = registry.set<::gui>(std::make_unique<::gui>(registry));
  auto editor_tab_manager = registry.set<::editor_tab_manager>(std::make_unique<::editor_tab_manager>(registry));

  load_assets(*assets_filesystem, *assets_repository, { ".entity", ".shader", ".schema", ".texture", ".dcc_asset" });

  // TODO
  create_and_compile_texture_asset("assets/textures/container.jpg", "assets/textures/container.texture", *assets_repository, *assets_filesystem);
  create_and_compile_texture_asset("assets/textures/seal.png", "assets/textures/seal.texture", *assets_repository, *assets_filesystem);

  compile_shaders(*assets_repository, *assets_filesystem);

  constexpr const char* backpack_asset_path = "assets/scenes/backpack.dcc_asset";
  constexpr const char* backpack_entity_path = "assets/scenes/backpack.entity";
  if (asset_id id = create_dcc_asset(fs::to_project_path("assets/scenes/backpack/backpack.obj"), *assets_repository)) {
    if (auto* asset = assets_repository->get_asset_by_path(backpack_asset_path)) {
      assets_repository->destroy_asset(asset->id());
    }

    assets_repository->set_asset_path(id, backpack_asset_path);
    assets_filesystem->save(*assets_repository, backpack_asset_path, true);
  }

  if (asset_id id = create_entity_from_dcc_asset(*assets_repository->get_asset_by_path(backpack_asset_path), *assets_repository, *assets_filesystem)) {
    if (auto* asset = assets_repository->get_asset_by_path(backpack_entity_path)) {
      assets_repository->destroy_asset(asset->id());
    }

    assets_repository->set_asset_path(id, backpack_entity_path);
    assets_filesystem->save(*assets_repository, backpack_entity_path, true);
  }

  schema_builder(meta::get_typeid<vec3>())
      .add("x", schema_type::FLOAT)
      .add("y", schema_type::FLOAT)
      .add("z", schema_type::FLOAT)
    .build(*schema_registry);

  schema_builder(meta::get_typeid<quat>())
      .add("x", schema_type::FLOAT)
      .add("y", schema_type::FLOAT)
      .add("z", schema_type::FLOAT)
      .add("w", schema_type::FLOAT)
    .build(*schema_registry);

  schema_builder(meta::get_typeid<transform>())
      .add("position", schema_type::OBJECT, meta::get_typeid<vec3>())
      .add("rotation", schema_type::OBJECT, meta::get_typeid<quat>())
      .add("scale", schema_type::OBJECT, meta::get_typeid<vec3>())
    .build(*schema_registry);

  schema_builder(meta::get_typeid<color>())
      .add("r", schema_type::FLOAT)
      .add("g", schema_type::FLOAT)
      .add("b", schema_type::FLOAT)
      .add("a", schema_type::FLOAT)
    .build(*schema_registry);

  build_schema_from_asset(meta::get_typeid<transform_component>(), *assets_repository->get_asset_by_path("schemas/transform_component.schema"), *schema_registry);
  build_schema_from_asset(meta::get_typeid<mesh_component>(), *assets_repository->get_asset_by_path("schemas/mesh_component.schema"), *schema_registry);

  window window({512, 512});

  swap_chain swap_chain = renderer->create_swap_chain(window.get_handle());

  auto resource_command_buffer = renderer->create_resource_command_buffer();
  shader_repository->compile(*assets_repository->get_asset_by_path("assets/shaders/TestShader.shader"), assets_repository.get(), resource_command_buffer.get());
  shader_repository->compile(*assets_repository->get_asset_by_path("assets/shaders/GUIShader.shader"), assets_repository.get(), resource_command_buffer.get());
  shader_repository->compile(*assets_repository->get_asset_by_path("assets/shaders/EditorGrid.shader"), assets_repository.get(), resource_command_buffer.get());
  renderer->submit(*resource_command_buffer);

  init_world(registry);
  init_render_pipeline(registry);
  register_components(registry);

  fs::path libs_folder = fs::to_project_path(".ubik/libs");
  library_loader libs;
  for (const auto& entry : fs::directory_iterator(libs_folder)) {
    if (!entry.is_directory() && entry.path().extension() == ".dylib")
      libs.load<systems_registry&>(entry.path().filename().c_str(), entry.path(), registry);
  }

  auto app = registry.get<application>();
  app->start();

  gui_renderer->init(&window);
  connect_gui_events(*gui_renderer, *input_system);

  world world;

  auto simulation_view = registry.view<simulation>();

  for (auto& sim : simulation_view) {
    sim->start(world);
  }

  timer timer;

  bool running = true;
  while (running) {

    libs.check_hot_reload<systems_registry&>(registry);

    window.update();

    window_event event;
    while (window.poll_event(event)) {
      if (event.type() == event_type::Close) {
        running = false;
      } else if (event.type() == event_type::Resize) {
        renderer->resize_swap_chain(swap_chain, static_cast<const resize_event&>(event).resolution);
      }
      input_system->push_event(event);
    }

    app->tick();

    float dt = timer.restart().as_milliseconds();
    for (auto& sim : simulation_view) {
      sim->update(world, dt);
    }

    viewer_registry->clear();

    renderer->begin_frame();

    gui_renderer->begin();

    editor_tab_manager->update();

    render_pipeline->render(viewer_registry->begin(), viewer_registry->end(), *renderer);

    window.set_cursor(gui_renderer->cursor());

    gui_renderer->end(swap_chain.back_buffer_handle);

    renderer->swap(swap_chain);
  }

  for (auto& sim : simulation_view) {
    sim->stop();
  }

  disconnect_gui_events(*gui_renderer, *input_system);

  app->stop();

}
