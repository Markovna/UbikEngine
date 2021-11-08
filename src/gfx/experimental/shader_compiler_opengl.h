#pragma once

#include "shader_compiler.h"

#include <memory>

namespace experimental {

class shader_compiler_opengl : public shader_compiler {
 public:
  static std::unique_ptr<shader_compiler_opengl> create();

  std::string_view name() const override;
  void compile(const asset&, std::ostream&) override;
};

}