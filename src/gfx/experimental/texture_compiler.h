#pragma once

#include "asset_repository.h"
#include <unordered_set>

void compile_textures(
    const std::unordered_set<std::string>& extensions,
    assets_repository& repository,
    assets_filesystem& assets);


