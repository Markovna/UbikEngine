#include <base/crc32.h>
#include "shader_repository.h"

#include "gfx/shader.h"
#include "core/renderer.h"
#include "core/assets_filesystem.h"
#include "gfx/shader_compiler.h"
#include "core/asset_repository.h"
#include "base/log.h"

#include <fstream>

shader_compile_result shader_repository::compile_stage(
    const asset_buffer& buffer, shader_stage::type stage) {
  std::string_view vs_source { reinterpret_cast<const char*>(buffer.data()), buffer.size() };
  return compiler_->compile(vs_source, stage);
}

void shader_repository::compile(const asset &asset, asset_repository *repository, resource_command_buffer *res_buf) {
  shader_program_desc program_desc {};

  program_desc.vertex = compile_stage(repository->load_buffer(asset.at("vertex_source")), shader_stage::VERTEX);
  program_desc.fragment = compile_stage(repository->load_buffer(asset.at("fragment_source")), shader_stage::FRAGMENT);

  key_t key = shaders_.emplace(std::make_unique<shader>(std::move(program_desc), res_buf));
  name_index_.emplace(asset.at("name"), key);
}

shader *shader_repository::lookup(std::string_view name) {
  auto it = name_index_.find(std::string(name));
  if (it == name_index_.end()) return nullptr;

  return shaders_.at(it->second).get();
}

uint32_t get_hash(const fs::path& path) {
  std::ifstream file(path, std::ios::binary | std::ios::in);
  return utils::crc32(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
}

void compile_shaders(asset_repository& repository, assets_filesystem& assets) {
  for (auto it = fs::recursive_directory_iterator(fs::project_path()); it != fs::recursive_directory_iterator(); it++) {
    if (it->is_directory())
      continue;

    if (it->path().extension() != ".vert")
      continue;

    fs::path p = it->path().lexically_relative(fs::project_path());
    compile_shader(
        p.replace_extension(""),
        repository,
        assets
    );
  }
}

void compile_shader(const fs::path& path, asset_repository& assets, assets_filesystem& assets_filesystem) {
  fs::path vert_path { fs::concat(path, ".vert") };
  fs::path frag_path { fs::concat(path, ".frag") };
  fs::path asset_path { fs::concat(path, ".shader") };

  auto asset = assets.get_asset(asset_path);
  if (!asset) {
    asset = &assets.create_asset();
    assets.set_asset_path(asset->id(), asset_path);
    assets.set_asset_guid(asset->id(), guid::generate());
  }

  if (auto it = asset->find("name"); it == asset->end()) {
    assets.set_value(*asset, "name", path.filename());
  }

  bool need_compile = false;
  if (auto it_vert = asset->find("vertex_source"), it_frag = asset->find("fragment_source");
      it_vert != asset->end() && it_frag != asset->end() &&
      it_vert->second.is_buffer() && it_frag->second.is_buffer()) {

    auto vert_buffer = static_cast<buffer_id>(it_vert->second);
    auto frag_buffer = static_cast<buffer_id>(it_frag->second);

    need_compile |= assets.buffer_hash(vert_buffer) != get_hash(fs::append(fs::project_path(), vert_path));
    need_compile |= assets.buffer_hash(frag_buffer) != get_hash(fs::append(fs::project_path(), frag_path));
  } else {
    need_compile = true;
  }

  if (need_compile) {
    logger::core::Info("Compile shader {}", path.c_str());

    {
      assets.set_value(*asset, "vertex_source", assets.create_buffer_from_file(vert_path, 0, 0));
      assets.set_value(*asset, "fragment_source", assets.create_buffer_from_file(frag_path, 0, 0));

      assets_filesystem.save(assets, asset_path);
    }

    assets_filesystem.load(assets, asset_path);
  }

}
