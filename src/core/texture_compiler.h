#pragma once

#include <unordered_set>
#include "platform/file_system.h"

struct asset_repository;
struct asset;
struct asset_id;
struct assets_filesystem;
struct guid;

asset& create_texture_asset(const fs::path& source_path, asset_repository& repository, guid guid);
void compile_texture_asset(asset& asset, asset_repository& repository);

void create_and_compile_texture_asset(
    const fs::path& source_path, const fs::path& path, asset_repository&, assets_filesystem&);

asset* create_texture_from_dcc_asset(
    const asset& dcc_asset, const fs::path& path, asset_repository& repository, assets_filesystem& filesystem);