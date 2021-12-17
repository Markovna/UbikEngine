#pragma once

#include "gfx.h"
#include "base/slot_map.h"
#include "shader.h"
#include "core/assets/asset.h"
#include "platform/file_system.h"

struct assets_repository;
struct assets_filesystem;
struct shader_compiler;

class shader_repository {
 private:

  template<class... Args>
  using container = stdext::slot_map<Args...>;

  using key_t = container<struct any>::key_type;

 public:
  explicit shader_repository(shader_compiler* compiler) : compiler_(compiler) {}

  shader_compile_result compile_stage(uint64_t buffer_id, assets_repository* repository, shader_stage::type stage);

  void compile(asset& asset, assets_repository* repository, resource_command_buffer* res_buf);

  shader* lookup(std::string_view name);

 private:
  container<std::unique_ptr<shader>> shaders_;
  std::unordered_map<std::string, key_t> name_index_;
  shader_compiler* compiler_;
};

void compile_shaders(assets_repository&, assets_filesystem&);
void compile_shader(const fs::path& path, assets_repository&, assets_filesystem&);
