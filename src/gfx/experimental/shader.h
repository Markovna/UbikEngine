#pragma once

#include <unordered_map>
#include <string>
#include "gfx.h"

namespace experimental {

class shader {
 public:
  shader();

 private:
  uint32_t attribute_locations_[gfx::vertex_semantic::COUNT];
  std::unordered_map<std::string, gfx::uniform_desc> uniform_locations_;
};

}