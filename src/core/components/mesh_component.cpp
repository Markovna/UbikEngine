#include "mesh_component.h"
#include "transform_component.h"

#include "core/renderer.h"
#include "core/render_pipeline.h"
#include "core/texture.h"
#include "gfx/command_buffers.h"
#include "gfx/shader_repository.h"
#include "gfx/vertex_layout_desc.h"

#include "core/meta/registration.h"
#include "core/meta/interface_registry.h"
#include "core/asset_repository.h"
#include "core/systems_registry.h"
#include "core/engine_events.h"
#include "core/component_loader.h"

static systems_registry* reg;

void load_mesh_component(const asset& asset, world& world, entity& e) {
  static float size = 1.0f;
  static float hs = size;
  static float vertices[] = {
      // pos          // tex coords
      -hs, -hs, -hs,  0.0f, 0.0f,
      hs, -hs, -hs,  1.0f, 0.0f,
      hs,  hs, -hs,  1.0f, 1.0f,
      hs,  hs, -hs,  1.0f, 1.0f,
      -hs,  hs, -hs,  0.0f, 1.0f,
      -hs, -hs, -hs,  0.0f, 0.0f,

      -hs, -hs,  hs,  0.0f, 0.0f,
      hs, -hs,  hs,  1.0f, 0.0f,
      hs,  hs,  hs,  1.0f, 1.0f,
      hs,  hs,  hs,  1.0f, 1.0f,
      -hs,  hs,  hs,  0.0f, 1.0f,
      -hs, -hs,  hs,  0.0f, 0.0f,

      -hs,  hs,  hs,  1.0f, 0.0f,
      -hs,  hs, -hs,  1.0f, 1.0f,
      -hs, -hs, -hs,  0.0f, 1.0f,
      -hs, -hs, -hs,  0.0f, 1.0f,
      -hs, -hs,  hs,  0.0f, 0.0f,
      -hs,  hs,  hs,  1.0f, 0.0f,

      hs,  hs,  hs,  1.0f, 0.0f,
      hs,  hs, -hs,  1.0f, 1.0f,
      hs, -hs, -hs,  0.0f, 1.0f,
      hs, -hs, -hs,  0.0f, 1.0f,
      hs, -hs,  hs,  0.0f, 0.0f,
      hs,  hs,  hs,  1.0f, 0.0f,

      -hs, -hs, -hs,  0.0f, 1.0f,
      hs, -hs, -hs,  1.0f, 1.0f,
      hs, -hs,  hs,  1.0f, 0.0f,
      hs, -hs,  hs,  1.0f, 0.0f,
      -hs, -hs,  hs,  0.0f, 0.0f,
      -hs, -hs, -hs,  0.0f, 1.0f,

      -hs,  hs, -hs,  0.0f, 1.0f,
      hs,  hs, -hs,  1.0f, 1.0f,
      hs,  hs,  hs,  1.0f, 0.0f,
      hs,  hs,  hs,  1.0f, 0.0f,
      -hs,  hs,  hs,  0.0f, 0.0f,
      -hs,  hs, -hs,  0.0f, 1.0f
  };


  auto renderer = reg->get<::renderer>();
  auto cmd_buf = renderer->create_resource_command_buffer();

  auto& comp = world.get<mesh_component>(e.id);

  memory vertex_mem;
  comp.vb = cmd_buf->create_vertex_buffer(vertex_layout_desc()
                                              .add(vertex_semantic::POSITION, vertex_type::FLOAT, 3)
                                              .add(vertex_semantic::TEXCOORD0, vertex_type::FLOAT, 2),
                                          sizeof(vertices),
                                          vertex_mem
  );

  std::memcpy(vertex_mem.data, vertices, sizeof(vertices));

  comp.uniform = cmd_buf->create_uniform(
      {
          { .type = uniform_type::SAMPLER, .binding = 0 },
          { .type = uniform_type::SAMPLER, .binding = 1 },
          { .type = uniform_type::BUFFER,  .binding = 0 },
          { .type = uniform_type::BUFFER,  .binding = 1 },
      });

  comp.model_buffer = cmd_buf->create_uniform_buffer(sizeof(mat4));
  comp.camera_buffer = cmd_buf->create_uniform_buffer(sizeof(view_projection));

  static auto rep = reg->get<asset_repository>();
  static texture texture0 = load_texture(*rep->get_asset("assets/textures/container.jpg.meta"), rep.get(), cmd_buf.get());
  static texture texture1 = load_texture(*rep->get_asset("assets/textures/seal.png.meta"), rep.get(), cmd_buf.get());

  cmd_buf->set_uniform(comp.uniform, 0, texture0.handle());
  cmd_buf->set_uniform(comp.uniform, 1, texture1.handle());
  cmd_buf->set_uniform(comp.uniform, 0, comp.camera_buffer);
  cmd_buf->set_uniform(comp.uniform, 1, comp.model_buffer);

  renderer->submit(*cmd_buf);
}

void render_mesh(
    view& view,
    const ecs::details::component_pool<transform_component>* transforms,
    const ecs::details::component_pool_base* components,
    renderer& renderer,
    render_command_buffer& render_commands,
    resource_command_buffer& resource_commands) {

  auto component_view = ecs::component_view<const transform_component, const mesh_component> {
      *transforms,
      *static_cast<const ecs::details::component_pool<mesh_component>*>(components)
  };

  for (ecs::entity e : component_view) {
    auto& mesh = component_view.get<const mesh_component>(e);
    auto& transform = component_view.get<const transform_component>(e);

    memory camera_uniform_mem;
    resource_commands.update_uniform_buffer(mesh.camera_buffer, sizeof(view_projection), camera_uniform_mem);
    std::memcpy(camera_uniform_mem.data, &view.camera, sizeof(view.camera));

    mat4 model = (mat4) transform.world;
    memory model_mem;
    resource_commands.update_uniform_buffer(mesh.model_buffer, sizeof(mat4), model_mem);
    std::memcpy(model_mem.data, &model, sizeof(model));

    render_commands.draw({
                             .sort_key = view.sort_key,
                             .shader = reg->get<shader_repository>()->lookup("TestShader")->handle(),
                             .vertexbuf = mesh.vb,
                             .uniforms = { mesh.uniform }
                         });
  }
}

void register_mesh_component(systems_registry& registry) {
  reg = &registry;

  auto type = meta::registration::type<mesh_component>();
  auto interface_registry = registry.get<::interface_registry>();
  interface_registry->register_interface(type.id(), load_component_interface(load_mesh_component));
  interface_registry->register_interface(type.id(), instantiate_component_interface(instantiate_component<mesh_component>));
  interface_registry->register_interface(type.id(), render_interface(render_mesh));
}

