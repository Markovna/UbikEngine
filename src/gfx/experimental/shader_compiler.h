#pragma once

#include <iostream>

#include "core/assets/assets.h"
#include "gfx/experimental/asset_repository.h"

namespace experimental {

class shader_compiler {
 public:
  virtual std::string_view name() const = 0;
  virtual void compile(const asset&, std::ostream&) = 0;
};

void compile_shader(std::string_view type, const asset& src, asset& dst, shader_compiler* compiler, assets_repository* assets) {
  std::stringstream os;
  compiler->compile(src, os);

  auto end = os.seekg(0, std::ios::end).tellg();
  auto beg = os.seekg(0, std::ios::beg).tellg();

  auto [buffer_id, buffer_data] = assets->buffers().add(end - beg);
  os.read((char*) buffer_data, end - beg);

  dst["__buffers"].push_back(buffer_id);
  dst[type.data()][compiler->name().data()] = buffer_id;
}

void compile_program(const asset& src, asset& dst, shader_compiler* compiler, assets_repository* assets) {
  dst["name"] = src["name"];

  compile_shader("vertex", src["vertex"], dst, compiler, assets);
  compile_shader("fragment", src["fragment"], dst, compiler, assets);

  for (auto& asset : src["vertex"]["bindings"]) {
    dst["bindings"].push_back(
    {
      {"name", asset["name"]},
      {"binding", asset["binding"]},
      {"type", asset["type"]},
      {"offset", asset["offset"]}
    });
  }
}

}