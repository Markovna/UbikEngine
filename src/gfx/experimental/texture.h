#pragma once

#include <stb_image.h>
#include "asset_repository.h"
#include "gfx.h"
#include "command_buffers.h"
#include "base/crc32.h"

class texture {
 public:
  texture(const texture_desc& desc, const uint8_t* data, size_t size, resource_command_buffer* command_buf);

  texture_handle handle() const { return handle_; }

 private:
  texture_handle handle_;
};

void compile_texture(const fs::path& source_path, const fs::path& asset_path, assets_repository& repository, assets_filesystem& assets);

texture load_texture(const asset& asset, assets_repository* repository, resource_command_buffer* command_buf);
