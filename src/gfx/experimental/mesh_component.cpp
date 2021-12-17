#include "mesh_component.h"
#include "gfx/experimental/render_pipeline.h"
#include "gfx/experimental/vertex_layout_desc.h"
#include <base/color.h>
#include "texture.h"
#include "asset_repository.h"
#include "renderer.h"
#include "command_buffers.h"
#include "core/meta/registration.h"
#include "gfx/experimental/systems_registry.h"
#include "gfx/experimental/shader_repository.h"
#include "core/engine_events.h"
#include "core/component_loader.h"

static systems_registry* reg;

void load_camera_component(const asset& asset, world& world, entity& e) {
  auto& comp = world.get<camera_component>(e.id);
  comp.fov = asset.at("fov");
  comp.near = asset.at("near");
  comp.far = asset.at("far");
  comp.orthogonal_size = asset.at("orthogonal_size");

  comp.normalized_rect.x = asset.at("normalized_rect").at("x");
  comp.normalized_rect.y = asset.at("normalized_rect").at("y");
  comp.normalized_rect.z = asset.at("normalized_rect").at("z");
  comp.normalized_rect.w = asset.at("normalized_rect").at("w");

  comp.clear_color = color::black();
}

void load_transform_component(const asset& asset, world& world, entity& e) {
  auto& comp = world.get<transform_component>(e.id);

  auto& position = asset.at("position");
  comp.local.position.x = position.at("x");
  comp.local.position.y = position.at("y");
  comp.local.position.z = position.at("z");

  auto& rotation = asset.at("rotation");
  comp.local.rotation.x = rotation.at("x");
  comp.local.rotation.y = rotation.at("y");
  comp.local.rotation.z = rotation.at("z");
  comp.local.rotation.w = rotation.at("w");

  auto& scale = asset.at("scale");
  comp.local.scale.x = scale.at("x");
  comp.local.scale.y = scale.at("y");
  comp.local.scale.z = scale.at("z");

  comp.dirty = true;
}

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
  resource_command_buffer* cmd_buf = renderer->create_resource_command_buffer();

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
  comp.camera_buffer = cmd_buf->create_uniform_buffer(sizeof(camera_view_projection));

  static auto rep = reg->get<assets_repository>();
  static texture texture0 = load_texture(*rep->load("assets/textures/container.jpg.meta"), rep.get(), cmd_buf);
  static texture texture1 = load_texture(*rep->load("assets/textures/seal.png.meta"), rep.get(), cmd_buf);

  cmd_buf->set_uniform(comp.uniform, 0, texture0.handle());
  cmd_buf->set_uniform(comp.uniform, 1, texture1.handle());
  cmd_buf->set_uniform(comp.uniform, 0, comp.camera_buffer);
  cmd_buf->set_uniform(comp.uniform, 1, comp.model_buffer);

  renderer->submit(cmd_buf);
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
    resource_commands.update_uniform_buffer(mesh.camera_buffer, sizeof(camera_view_projection), camera_uniform_mem);
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



void load_mesh(systems_registry& registry) {
  reg = &registry;

  meta::registration::type<mesh_component>();
  meta::registration::type<transform_component>();
  meta::registration::type<camera_component>();

  meta::registration::interface<
      mesh_component,
      component_loader>({
        .from_asset = load_mesh_component,
        .instantiate = instantiate_component<mesh_component>
      });

  meta::registration::interface<
      transform_component,
      component_loader>({
        .from_asset = load_transform_component,
        .instantiate = instantiate_component<transform_component>
      });

  meta::registration::interface<
      camera_component,
      component_loader>({
        .from_asset = load_camera_component,
        .instantiate = instantiate_component<camera_component>
      });

  meta::registration::interface<
      mesh_component,
      render_i>({
          .render = render_mesh
      });
}

