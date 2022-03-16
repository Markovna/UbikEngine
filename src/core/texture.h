#pragma once

#include "gfx/gfx.h"

struct resource_command_buffer;
struct asset_repository;
struct assets_filesystem;
struct asset;

class texture {
 public:
  texture(texture_handle handle) : handle_(handle) {}
  texture_handle handle() const { return handle_; }

 private:
  texture_handle handle_;
};

texture load_texture(const asset& asset, asset_repository* repository, resource_command_buffer* command_buf);
