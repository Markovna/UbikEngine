#pragma once

#include "platform/file_system.h"

class asset_repository;
class asset;

class assets_filesystem {
 public:
  assets_filesystem() noexcept = default;

  void save(asset_repository&, const fs::path&, bool remap_buffers);
  void load(asset_repository&, const fs::path&) const;

 private:
  void save(asset_repository&, asset&, const fs::path&, bool remap_buffers);
};

void load_assets(const assets_filesystem&, asset_repository&, std::initializer_list<fs::path> extensions = {});
void save_assets(assets_filesystem&, asset_repository&, bool remap_buffers);