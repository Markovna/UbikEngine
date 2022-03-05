#pragma once

#include <unordered_set>
#include "platform/file_system.h"

struct asset_repository;
struct assets_filesystem;

void compile_textures(
    const std::unordered_set<std::string>& extensions,
    asset_repository& repository,
    assets_filesystem& assets);

void compile_texture(
    const fs::path& source_path,
    const fs::path& asset_path,
    asset_repository& repository,
    assets_filesystem& assets);

