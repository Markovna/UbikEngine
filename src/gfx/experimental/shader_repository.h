#pragma once

#include "shader.h"
#include "renderer.h"
#include "gfx/experimental/asset_repository.h"

namespace experimental {

class shader_repository {
 private:
  struct shader_item {
    std::unique_ptr<shader> ptr = nullptr;
  };

  using container = stdext::slot_map<shader_item>;

 public:
  shader_repository() = default;

  void load(const asset& asset, assets_repository* assets, gfx::renderer* renderer) {
    gfx::shader_program_desc shader_desc {};

    uint64_t vs_buffer_id = asset.at("vertex").at(renderer->backend_name().data());
    auto [vs_size, vs_data] = assets->buffers().get(vs_buffer_id);
    shader_desc.vertex_shader.data = vs_data;
    shader_desc.vertex_shader.size = vs_size;

    uint64_t fs_buffer_id = asset.at("fragment").at(renderer->backend_name().data());
    auto [fs_size, fs_data] = assets->buffers().get(fs_buffer_id);
    shader_desc.fragment_shader.data = fs_data;
    shader_desc.fragment_shader.size = fs_size;

    gfx::resource_command_buffer* res_buf = renderer->create_resource_command_buffer();
    gfx::shader_handle shader_handle = res_buf->create_shader(std::move(shader_desc));

    shader_binding_table binding_table;
    for (auto& binding : asset.at("bindings")) {
      shader_binding_table::item& item = binding_table.items.emplace_back();
      item.binding = binding.at("binding");
      item.type = binding.at("type");
      item.offset = binding.at("offset");

      const std::string& binding_name = binding.at("name");
      std::strcpy(item.name, binding_name.c_str());
    }

    container::key_type key = shaders_.emplace(shader_item {
        .ptr = std::make_unique<shader>(shader_handle, binding_table)
    });
    name_index_.emplace(asset.at("name"), key);
  }

  shader* lookup(std::string_view name) {
    auto it = name_index_.find(std::string(name));
    if (it == name_index_.end()) return nullptr;

    return shaders_.at(it->second).ptr.get();
  }

 private:
  container shaders_;
  std::unordered_map<std::string, container::key_type> name_index_;
};

}