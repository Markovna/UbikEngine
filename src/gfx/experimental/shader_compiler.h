#pragma once

#include <iostream>

#include "core/assets/assets.h"
#include "gfx/experimental/asset_repository.h"

namespace experimental {

struct shader_reflection {};

struct shader_stage {
  enum type {
    VERTEX = 0,
    FRAGMENT,

    COUNT
  };
};

class shader_compiler {
 public:
  virtual bool compile(std::string_view, shader_stage::type, shader_reflection&, std::ostream&) = 0;
};

}