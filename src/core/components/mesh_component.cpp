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

void load_mesh_component(const asset& component_asset, world& world, entity& e) {
  auto renderer = reg->get<::renderer>();
  auto rep = reg->get<asset_repository>();
  auto cmd_buf = renderer->create_resource_command_buffer();

  auto& comp = world.get<mesh_component>(e.id);
  guid mesh_guid = guid::from_string(component_asset.at("mesh"));

  asset* mesh_asset = rep->get_asset(mesh_guid);
  auto &attributes = mesh_asset->at("attributes").get<asset_array &>();
  auto *indices_buffer = rep->get_asset(guid::from_string(mesh_asset->at("indices")));

  uint32_t vertex_buffer_size = 0;
  uint32_t stride = 0;
  vertex_layout_desc vertex_layout = {};
  for (const asset &attr : attributes) {
    vertex_semantic::type semantic = (vertex_semantic::type) (uint32_t) attr.at("semantic");
    asset *accessor = rep->get_asset(guid::from_string(attr.at("accessor")));

    vertex_type::type type = vertex_type::COUNT;
    uint32_t size = accessor->at("size");
    uint32_t count = accessor->at("count");
    uint32_t components = accessor->at("components");
    bool unsigned_ = accessor->contains("unsigned") && accessor->at("unsigned");
    bool is_float = accessor->contains("float") && accessor->at("float");
    if (is_float && size == 4) {
      type = vertex_type::FLOAT;
    } else if (!is_float) {
      if (size == 1) {
        type = unsigned_ ? vertex_type::UINT8 : vertex_type::INT8;
      } else if (size == 2) {
        type = unsigned_ ? vertex_type::UINT16 : vertex_type::INT16;
      } else if (size == 4) {
        type = unsigned_ ? vertex_type::UINT32 : vertex_type::INT32;
      }
    }

    if (type == vertex_type::COUNT) {
      logger::core::Error("Parse attribute type failed (size: {}, float: {}, unsigned: {}).",
                          size,
                          is_float,
                          unsigned_);
    }

    vertex_buffer_size += count * components * size;
    stride += components * size;

    vertex_layout.add(semantic, type, components, semantic == vertex_semantic::NORMAL);
  }

  uint8_t vertex_buffer[vertex_buffer_size];
  size_t attribute_offset = 0;
  for (const asset &attr : attributes) {
    size_t vertex_buffer_offset = 0;
    asset *accessor = rep->get_asset(guid::from_string(attr.at("accessor")));
    asset* buffer_asset = rep->get_asset(guid::from_string(accessor->at("buffer")));
    asset_buffer buf = rep->load_buffer(buffer_asset->at("data"));
    uint32_t size = accessor->at("size");
    uint32_t count = accessor->at("count");
    uint32_t components = accessor->at("components");
    size_t offset = accessor->contains("offset") ? (size_t) accessor->at("offset") : 0;

    for (size_t i = 0; i < count; ++i) {
      std::memcpy(vertex_buffer + vertex_buffer_offset + attribute_offset,
                  buf.data() + offset + i * size * components,
                  size * components);
      vertex_buffer_offset += stride;
    }

    attribute_offset += size * components;
  }

  memory vertex_mem;
  comp.vb = cmd_buf->create_vertex_buffer(
      vertex_layout,
      vertex_buffer_size,
      vertex_mem
  );
  std::memcpy(vertex_mem.data, vertex_buffer, vertex_buffer_size);

  asset *indices_accessor = rep->get_asset(guid::from_string(mesh_asset->at("indices")));
  asset* buffer_asset = rep->get_asset(guid::from_string(indices_accessor->at("buffer")));
  asset_buffer indices_buf = rep->load_buffer(buffer_asset->at("data"));
  uint32_t indices_size = indices_accessor->at("size");
  uint32_t indices_count = indices_accessor->at("count");
  uint32_t indices_components = indices_accessor->at("components");
  size_t indices_offset = indices_accessor->contains("offset") ? (size_t) indices_accessor->at("offset") : 0;

  memory index_mem;
  comp.ib = cmd_buf->create_index_buffer(indices_count * indices_size * indices_components, index_mem);
  std::memcpy(index_mem.data, indices_buf.data() + indices_offset, index_mem.size);

  comp.uniform = cmd_buf->create_uniform(
      {
          { .type = uniform_type::SAMPLER, .binding = 0 },
          { .type = uniform_type::SAMPLER, .binding = 1 },
          { .type = uniform_type::BUFFER,  .binding = 0 },
          { .type = uniform_type::BUFFER,  .binding = 1 },
      });

  comp.model_buffer = cmd_buf->create_uniform_buffer(sizeof(mat4));
  comp.camera_buffer = cmd_buf->create_uniform_buffer(sizeof(view_projection));

  if (component_asset.contains("texture")) {
    guid texture_guid = guid::from_string(component_asset.at("texture"));

    // TODO
    static std::unordered_map<guid, std::unique_ptr<texture>> textures;

    auto& ptr = textures[texture_guid];
    if (!ptr) {
      ptr = std::make_unique<texture>(load_texture(*rep->get_asset(texture_guid), rep.get(), cmd_buf.get()));
    }

    cmd_buf->set_uniform(comp.uniform, 0, ptr->handle());
//    texture texture1 = load_texture(rep->get_asset("assets/textures/seal.texture").at("output"), rep.get(), cmd_buf.get());
//    cmd_buf->set_uniform(comp.uniform, 1, texture1.handle());
  }

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
                             .indexbuf = mesh.ib,
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

