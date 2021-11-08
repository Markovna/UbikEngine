#include "shader_compiler_opengl.h"

namespace experimental {

std::string_view shader_compiler_opengl::name() const {
  constexpr static const char* _name = "OpenGL";
  return _name;
}

void shader_compiler_opengl::compile(const asset& asset, std::ostream& out) {
  std::string source = asset["source"];
  out.write(source.data(), source.size());
}

std::unique_ptr<shader_compiler_opengl> shader_compiler_opengl::create() {
  return std::make_unique<shader_compiler_opengl>();
}

}