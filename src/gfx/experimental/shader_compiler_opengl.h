#pragma once

#include "shader_compiler.h"

#include <memory>

class shader_compiler_opengl : public shader_compiler {
 public:
  static std::unique_ptr<shader_compiler_opengl> create();

  shader_compile_result compile(std::string_view, shader_stage::type) override;
};
