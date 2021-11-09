#pragma once

#include "shader_compiler.h"

#include <memory>

namespace experimental {

class shader_compiler_opengl : public shader_compiler {
 public:
  static std::unique_ptr<shader_compiler_opengl> create();

  bool compile(std::string_view, shader_stage::type, shader_reflection&, std::ostream&) override;
};

}