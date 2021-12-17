#include <base/crc32.h>
#include "shader_repository.h"

#include "shader.h"
#include "renderer.h"
#include "gfx/experimental/asset_repository.h"
#include "gfx/experimental/shader_compiler.h"

shader_compile_result shader_repository::compile_stage(
    uint64_t buffer_id, assets_repository *repository, shader_stage::type stage) {
  auto [size, data] = repository->buffers().get(buffer_id);
  std::string_view vs_source { reinterpret_cast<const char*>(data.get()), size };
  return compiler_->compile(vs_source, stage);
}

void shader_repository::compile(asset &asset, assets_repository *repository, resource_command_buffer *res_buf) {
  shader_program_desc program_desc {};

  program_desc.vertex = compile_stage(asset.at("vertex_source").at("__buffer_id"), repository, shader_stage::VERTEX);
  program_desc.fragment = compile_stage(asset.at("fragment_source").at("__buffer_id"), repository, shader_stage::FRAGMENT);

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

void compile_shaders(assets_repository& repository, assets_filesystem& assets) {
  for (auto it = fs::recursive_directory_iterator(fs::current_path()); it != fs::recursive_directory_iterator(); it++) {
    if (it->is_directory())
      continue;

    if (it->path().extension() != ".vert")
      continue;

    fs::path p = it->path().lexically_relative(fs::current_path());
    compile_shader(
        p.replace_extension(""),
        repository,
        assets
    );
  }
}

void compile_shader(const fs::path& path, assets_repository& assets, assets_filesystem& assets_filesystem) {
  fs::path vert_path { fs::concat(path, ".vert") };
  fs::path frag_path { fs::concat(path, ".frag") };
  fs::path asset_path { fs::concat(path, ".shader") };

  auto asset = assets.load(asset_path);
  if (!asset) {
    guid guid = guid::generate();
    asset = assets.create_asset(guid, asset_path);
    (*asset)["__guid"] = guid;
  }

  if (!asset->contains("name")) {
    (*asset)["name"] = path.filename().c_str();
  }

  bool need_compile = false;
  if (asset->contains("vertex_source") && asset->at("vertex_source").contains("__buffer_id") &&
      asset->contains("fragment_source") && asset->at("fragment_source").contains("__buffer_id"))
  {
    uint64_t vert_buffer_id = asset->at("vertex_source").at("__buffer_id");
    uint64_t frag_buffer_id = asset->at("fragment_source").at("__buffer_id");

    need_compile |= assets.buffers().has(vert_buffer_id) && assets.buffers().hash(vert_buffer_id) != get_hash(fs::append(fs::current_path(), vert_path));
    need_compile |= assets.buffers().has(frag_buffer_id) && assets.buffers().hash(frag_buffer_id) != get_hash(fs::append(fs::current_path(), frag_path));
  } else {
    need_compile = true;
  }

  if (need_compile) {
    logger::core::Info("Compile shader {}", path.c_str());

    {
      auto vert_buf_id = assets.buffers().map(vert_path);
      auto frag_buf_id = assets.buffers().map(frag_path);
      (*asset)["vertex_source"] = {{"__buffer_id", vert_buf_id}};
      (*asset)["fragment_source"] = {{"__buffer_id", frag_buf_id}};

      assets_filesystem.save(assets, asset_path);
    }

    assets_filesystem.load(assets, asset_path);
  }

}
