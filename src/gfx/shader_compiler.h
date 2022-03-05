#pragma once

#include <iostream>

#include "gfx/gfx.h"

class shader_compiler {
 public:
  virtual shader_compile_result compile(std::string_view, shader_stage::type) = 0;
};
