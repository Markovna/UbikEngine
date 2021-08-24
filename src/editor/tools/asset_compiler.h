#pragma once

#include "platform/file_system.h"
#include "core/assets/asset.h"
#include "core/assets/texture.h"
#include "core/assets/shader.h"

namespace assets {

template<class T>
bool compile_asset(std::ifstream&, const asset&, std::ostream& output);

template<>
bool compile_asset<texture>(std::ifstream&, const asset&, std::ostream& output);

template<>
bool compile_asset<shader>(std::ifstream&, const asset&, std::ostream& output);

void compile_asset(const char* path);
void compile_all_assets(const char* directory);

}