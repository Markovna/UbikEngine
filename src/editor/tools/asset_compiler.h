#pragma once

#include "platform/file_system.h"
#include "core/assets/asset.h"
#include "core/assets/texture.h"
#include "core/assets/shader.h"

namespace assets {

template<class T>
bool compile_asset(const fs::path& path, const asset&, const fs::path& output_path);

template<>
bool compile_asset<texture>(const fs::path& path, const asset&, const fs::path& output_path);

template<>
bool compile_asset<shader>(const fs::path& path, const asset&, const fs::path& output_path);

void compile_asset(const char* path);

}