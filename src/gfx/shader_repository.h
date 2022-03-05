#pragma once

#include "gfx/gfx.h"
#include "base/slot_map.h"
#include "gfx/shader.h"
#include "platform/file_system.h"

struct assets_filesystem;
struct asset_repository;
struct shader_compiler;
struct asset_buffer;
struct asset;

class shader_repository {
 private:

  template<class... Args>
  using container = stdext::slot_map<Args...>;

  using key_t = container<struct any>::key_type;

 public:
  explicit shader_repository(shader_compiler* compiler) : compiler_(compiler) {}

  shader_compile_result compile_stage(const asset_buffer& buffer, shader_stage::type stage);

  void compile(const asset& asset, asset_repository* repository, resource_command_buffer* res_buf);

  shader* lookup(std::string_view name);

 private:
  container<std::unique_ptr<shader>> shaders_;
  std::unordered_map<std::string, key_t> name_index_;
  shader_compiler* compiler_;
};

void compile_shaders(asset_repository&, assets_filesystem&);
void compile_shader(const fs::path& path, asset_repository&, assets_filesystem&);
