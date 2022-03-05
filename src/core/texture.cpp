#include "texture.h"
#include "core/assets_filesystem.h"
#include "core/asset_repository.h"
#include "gfx/command_buffers.h"

texture load_texture(const asset &asset,
                     asset_repository *repository,
                     resource_command_buffer *command_buf) {
  uint32_t width = asset.at("width");
  uint32_t height = asset.at("height");
  uint32_t format = asset.at("format");
  buffer_id buf_id = asset.at("data");

  auto buffer = repository->load_buffer(buf_id);
  texture_desc desc = {
      .width = width,
      .height = height,
      .format = (texture_format::type) format,
  };

  return { desc, buffer.data(), buffer.size(), command_buf };
}

texture::texture(const texture_desc &desc, const uint8_t *data, size_t size, resource_command_buffer *command_buf) {
  memory mem;
  handle_ = command_buf->create_texture(desc, mem);
  std::memcpy(mem.data, data, size);
}
