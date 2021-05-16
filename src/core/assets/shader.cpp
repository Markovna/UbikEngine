#include "shader.h"


namespace assets {

template<>
std::unique_ptr<shader> load_asset(const std::istream& stream) {
  std::stringstream ss;
  ss << stream.rdbuf();
  std::string source(ss.str());
  return std::make_unique<shader>(source);
}

}

shader::shader(const std::string &source)
  : handle_(gfx::create_shader(source))
{}

shader::shader(shader&& other) noexcept
  : handle_()
{
  swap(other);
}

shader &shader::operator=(shader&& other) noexcept {
  shader(std::move(other)).swap(*this);
  return *this;
}

shader::~shader() {
  if (handle_)
    gfx::destroy(handle_);
}

void shader::swap(shader &other) {
  std::swap(handle_, other.handle_);
}


