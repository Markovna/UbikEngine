#include "texture.h"

texture load_texture(const asset &asset,
                     assets_repository *repository,
                     resource_command_buffer *command_buf) {
  uint32_t width = asset.at("width");
  uint32_t height = asset.at("height");
  uint32_t format = asset.at("format");
  uint64_t buffer_id = asset.at("data").at("__buffer_id");

  auto [size, data] = repository->buffers().get(buffer_id);
  texture_desc desc = {
      .width = width,
      .height = height,
      .format = (texture_format::type) format,
  };

  return {desc, data.get(), size, command_buf};
}

void compile_texture(const fs::path &source_path,
                     const fs::path &asset_path,
                     assets_repository& repository,
                     assets_filesystem& assets) {
  asset_handle handle = repository.load(asset_path);
  if (!handle) {
    guid guid = guid::generate();
    handle = repository.create_asset(guid, asset_path);
    (*handle)["__guid"] = guid;
  }

  asset& asset = *handle;

  bool need_compile = false;

  if (asset.contains("hash") &&
      asset.contains("width") &&
      asset.contains("height") &&
      asset.contains("format") &&
      asset.contains("data") &&
      asset.at("data").contains("__buffer_id")) {

    uint64_t buffer_id = asset.at("data").at("__buffer_id");

    if (repository.buffers().has(buffer_id)) {
      std::ifstream file(fs::append(fs::current_path(), source_path), std::ios::binary | std::ios::in);
      uint32_t hash = utils::crc32(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());

      need_compile |= asset.at("hash") != hash;
    } else {
      need_compile = true;
    }
  } else {
    need_compile = true;
  }

  if (need_compile) {
    logger::core::Info("Compile texture {}", source_path.c_str());

    std::ifstream file(fs::append(fs::current_path(), source_path), std::ios::binary | std::ios::in);

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
      auto[buf_id, buf_data] = repository.buffers().add(buf_size);
      std::memcpy(buf_data.get(), data, buf_size);
      stbi_image_free(data);

      texture_format::type format = channels == 3 ? texture_format::RGB8 : texture_format::RGBA8;

      asset["width"] = width;
      asset["height"] = height;
      asset["format"] = format;
      asset["data"] = {
          {"__buffer_id", buf_id}
      };
      asset["hash"] = hash;

      assets.save(repository, asset_path);
    }

    // reload
    assets.load(repository, asset_path);
  }
}

texture::texture(const texture_desc &desc, const uint8_t *data, size_t size, resource_command_buffer *command_buf) {
  memory mem;
  handle_ = command_buf->create_texture(desc, mem);
  std::memcpy(mem.data, data, size);
}
