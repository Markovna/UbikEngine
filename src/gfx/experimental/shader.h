#pragma once

#include "gfx.h"
#include "core/assets/asset_loader.h"

#include <unordered_map>
#include <string>

namespace experimental {

struct shader_binding_table {
  struct item {
    char name[64];
    gfx::uniform_type::type type;
    uint8_t binding;
    uint32_t offset;
  };

  std::vector<item> items;
};

class shader {
 public:
  shader(gfx::shader_handle handle, shader_binding_table binding_table)
    : handle_(handle), binding_table_(std::move(binding_table))
  {}

 private:
  gfx::shader_handle handle_;
  shader_binding_table binding_table_;
};

}