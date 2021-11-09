#pragma once

#include "shader.h"
#include "renderer.h"
#include "gfx/experimental/asset_repository.h"
#include "gfx/experimental/shader_compiler.h"

namespace experimental {

class shader_repository {
 private:
  struct shader_item {
    std::unique_ptr<shader> ptr = nullptr;
  };

  using container = stdext::slot_map<shader_item>;

 public:
  shader_repository() = default;

  std::string compile_stage(std::string_view& source, shader_compiler* compiler, shader_stage::type stage, shader_reflection& reflection) {
    std::string compiled;
    std::ostringstream ostream { compiled };
    compiler->compile(source, stage, reflection, ostream);
    return compiled;
  }

  void compile(asset& asset, assets_repository* repository, shader_compiler* compiler, gfx::resource_command_buffer* res_buf) {
    gfx::shader_program_desc program_desc {};
    shader_reflection reflection {};

    {
      uint64_t vs_buffer_id = asset.at("vertex_source").at("__buffer_id");
      auto [size, data] = repository->buffers().get(vs_buffer_id);
      std::string_view source { reinterpret_cast<const char*>(data), size };
      std::string compiled = compile_stage(source, compiler, shader_stage::VERTEX, reflection);
      program_desc.vertex_shader.data = compiled.data();
      program_desc.vertex_shader.size = compiled.size();

      repository->buffers().unload(vs_buffer_id);
    }

    {
      uint64_t fs_buffer_id = asset.at("fragment_source").at("__buffer_id");
      auto [size, data] = repository->buffers().get(fs_buffer_id);
      std::string_view source { reinterpret_cast<const char*>(data), size };
      std::string compiled = compile_stage(source, compiler, shader_stage::FRAGMENT, reflection);
      program_desc.fragment_shader.data = compiled.data();
      program_desc.fragment_shader.size = compiled.size();

      repository->buffers().unload(fs_buffer_id);
    }

//    gfx::shader_handle shader_handle = res_buf->create_shader(std::move(program_desc));
//    container::key_type key = shaders_.emplace(shader_item {
//        .ptr = std::make_unique<shader>(shader_handle, reflection)
//    });
//    name_index_.emplace(asset.at("name"), key);
  }

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