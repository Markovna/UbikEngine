#include "texture_compiler.h"

#include "core/asset_repository.h"
#include "core/assets_filesystem.h"
#include "texture.h"
#include "base/log.h"
#include "base/crc32.h"
#include "dcc_asset.h"

#include <stb_image.h>
#include <fstream>

asset& create_texture_asset(const fs::path &source_path, asset_repository &repository, guid guid) {
  asset& texture = repository.create_asset(guid);
  asset& input = repository.create_asset();
  repository.set_value(texture, "input", input);
  repository.set_value(input, "path", source_path.c_str());

  asset& output = repository.create_asset();
  repository.set_value(texture, "output", output);
  repository.set_value(output, "buffer", repository.create_buffer(0));

  return texture;
}

void compile_texture_buffer(const uint8_t* buffer, size_t size, asset& texture, asset_repository& repository) {
  stbi_set_flip_vertically_on_load(true);

  int32_t width, height, channels, desired_channels = 0;
  uint8_t* data = stbi_load_from_memory(buffer, size, &width, &height, &channels, desired_channels);

  texture_data_desc desc { .width = (uint32_t) width, .height = (uint32_t) height, .format = channels == 3 ? texture_format::RGB8 : texture_format::RGBA8 };
  uint32_t desc_size = sizeof(texture_data_desc);
  uint32_t data_size = texture_size(desc);

  auto buf_id = repository.create_buffer(desc_size + data_size);

  repository.update_buffer(buf_id, 0, &desc, desc_size);
  repository.update_buffer(buf_id, desc_size, data, data_size);

  if (!texture.contains("output")) {
    repository.set_value(texture, "output", repository.create_asset());
  }

  repository.set_value(texture.at("output").get<asset&>().id(), "buffer", buf_id);

  stbi_image_free(data);
}

void compile_texture_asset(asset& texture, asset_repository& repository) {
  const std::string& source_path = texture.at("input").get<asset&>().at("path");
  logger::core::Info("Compile texture {}", source_path.c_str());

  std::ifstream file(fs::to_project_path(source_path), std::ios::binary | std::ios::in);

  size_t buf_size = file.rdbuf()->pubseekoff(0, std::ios::end, std::ios::in);
  char buffer[buf_size];
  file.rdbuf()->pubseekpos(0, std::ios_base::in);
  file.rdbuf()->sgetn(buffer, buf_size);

  compile_texture_buffer(reinterpret_cast<stbi_uc*>(buffer), buf_size, texture, repository);
}

asset* create_texture_from_dcc_asset(
    const asset& dcc_texture, const fs::path& texture_path, asset_repository& repository, assets_filesystem& filesystem) {

  dcc_asset_texture_type texture_type = (dcc_asset_texture_type) dcc_texture.at("type").get<uint32_t>();
  if (texture_type == dcc_asset_texture_type::UNKNOWN) {
    logger::core::Warning("Creating texture from dcc asset at path {0} failed: unknown texture type.", texture_path.c_str());
    return nullptr;
  }

  const std::string& name = dcc_texture.at("name");

  guid texture_asset_guid;
  if (asset *asset = repository.get_asset_by_path(texture_path)) {
    texture_asset_guid = repository.get_guid(asset->id());
    repository.destroy_asset(asset->id());
  } else {
    texture_asset_guid = guid::generate();
  }

  guid buffer_guid = guid::from_string(dcc_texture.at("buffer"));

  asset* buffer_asset = repository.get_asset(buffer_guid);
  if (!buffer_asset) {
    logger::core::Warning("Creating texture from dcc asset at path {0} failed: couldn't find buffer for texture {1}", texture_path.c_str(), name);
    return nullptr;
  }

  asset_buffer buffer = repository.load_buffer(buffer_asset->at("data"));

  asset& texture_asset = repository.create_asset(texture_asset_guid);
  repository.set_asset_path(texture_asset.id(), texture_path);

  logger::core::Info("Created texture from dcc asset at path {}, guid: {}", texture_path.c_str(), texture_asset_guid.str());

  if (texture_type != dcc_asset_texture_type::RAW) {
    compile_texture_buffer(buffer.data(), buffer.size(), texture_asset, repository);
  } else {
    auto buf_id = repository.create_buffer(buffer.size());
    repository.update_buffer(buf_id, 0, buffer.data(), buffer.size());

    if (!texture_asset.contains("output")) {
      repository.set_value(texture_asset, "output", repository.create_asset());
    }

    const asset& output = texture_asset.at("output");
    repository.set_value(output.id(), "buffer", buf_id);
  }

  filesystem.save(repository, texture_path, true);
  return &texture_asset;
}

void create_and_compile_texture_asset(const fs::path& source_path, const fs::path& path, asset_repository& repository, assets_filesystem& filesystem) {
  fs::path full_src_path(fs::to_project_path(source_path));
  if (!fs::exists(full_src_path)) {
    logger::core::Error("Couldn't create texture asset because source file on path {} doesn't exist.", source_path.c_str());
    return;
  }

  guid guid;
  if (asset* asset = repository.get_asset_by_path(path)) {
    guid = repository.get_guid(asset->id());
    repository.destroy_asset(asset->id());
  } else {
    guid = guid::generate();
  }

  asset& asset = create_texture_asset(source_path, repository, guid);
  repository.set_asset_path(asset.id(), path);

  compile_texture_asset(asset, repository);

  filesystem.save(repository, path, true);
}