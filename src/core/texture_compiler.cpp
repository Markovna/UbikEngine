#include "texture_compiler.h"

#include "core/asset_repository.h"
#include "core/assets_filesystem.h"
#include "texture.h"
#include "base/log.h"
#include "base/crc32.h"

#include <stb_image.h>
#include <fstream>

void compile_textures(
  const std::unordered_set<std::string>& extensions,
  asset_repository& repository,
  assets_filesystem& assets)
{
  for (auto it = fs::recursive_directory_iterator(fs::project_path());
        it != fs::recursive_directory_iterator();
        ++it) {

    if (it->is_directory())
      continue;

    if (!extensions.count(it->path().extension()))
      continue;

    fs::path p = it->path().lexically_relative(fs::project_path());
    compile_texture(
        it->path(),
        fs::concat(p, ".meta"),
        repository,
        assets
      );
  }
}

void compile_texture(const fs::path &source_path,
                     const fs::path &asset_path,
                     asset_repository& repository,
                     assets_filesystem& assets) {
  auto asset = repository.get_asset(asset_path);
  if (!asset) {
    asset = &repository.create_asset();
    repository.set_asset_path(asset->id(), asset_path);
    repository.set_asset_guid(asset->id(), guid::generate());
  }

  bool need_compile = false;

  if (asset->contains("hash") &&
      asset->contains("width") &&
      asset->contains("height") &&
      asset->contains("format") &&
      asset->contains("data") &&
      asset->at("data").is_buffer()) {

    std::ifstream file(fs::append(fs::project_path(), source_path), std::ios::binary | std::ios::in);

    auto buf_id = static_cast<buffer_id>(asset->at("data"));
    need_compile |= repository.buffer_hash(buf_id) != utils::crc32(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());

  } else {
    need_compile = true;
  }

  if (need_compile) {
    logger::core::Info("Compile texture {}", source_path.c_str());

    std::ifstream file(fs::append(fs::project_path(), source_path), std::ios::binary | std::ios::in);

    size_t src_size = file.rdbuf()->pubseekoff(0, std::ios::end, std::ios::in);
    char src_buffer[src_size];
    file.rdbuf()->pubseekpos(0, std::ios_base::in);
    file.rdbuf()->sgetn(src_buffer, src_size);

    stbi_set_flip_vertically_on_load(true);

    int32_t width, height, channels, desired_channels = 0;
    uint8_t* data = stbi_load_from_memory(reinterpret_cast<stbi_uc*>(src_buffer), src_size, &width, &height, &channels, desired_channels);

    file.rdbuf()->pubseekpos(0, std::ios_base::in);
    uint32_t hash = utils::crc32(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());

    {
      size_t buf_size = width * height * channels;
      auto buf_id = repository.create_buffer(data, buf_size);
      stbi_image_free(data);

      texture_format::type format = channels == 3 ? texture_format::RGB8 : texture_format::RGBA8;

      repository.set_value(asset->id(), "width", width);
      repository.set_value(asset->id(), "height", height);
      repository.set_value(asset->id(), "format", (int) format);
      repository.set_value(asset->id(), "data", buf_id);
      repository.set_value(asset->id(), "hash", hash);

      assets.save(repository, asset_path);
    }

    // reload
    assets.load(repository, asset_path);
  }
}