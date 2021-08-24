#pragma once

#include "platform/file_system.h"
#include "core/experimental2/assets_repository.h"

#include "core/assets/texture.h"
#include "core/assets/shader.h"

namespace experimental2::resources {

template<class T>
bool compile_asset(std::ifstream& stream, const assets::asset& settings, std::ostream&);

template<>
bool compile_asset<texture>(std::ifstream&, const assets::asset&, std::ostream& output);

template<>
bool compile_asset<shader>(std::ifstream&, const assets::asset&, std::ostream& output);

void compile(
    const fs::path& directory,
    const fs::path& dest,
    std::initializer_list<fs::path> ignore = {});

};


