#include "shader.h"

#include <sstream>

template<>
std::unique_ptr<shader> assets::loader::load(std::istream& stream) {
  gfx::attribute::binding_pack bindings;

  stream.read((char*) &bindings, sizeof(bindings));

  size_t vertex_size;
  stream.read((char*) &vertex_size, sizeof(vertex_size));

  char vertex_src[vertex_size];
  stream.read(vertex_src, vertex_size);

  size_t fragment_size;
  stream.read((char*) &fragment_size, sizeof(fragment_size));

  char fragment_src[fragment_size];
  stream.read(fragment_src, fragment_size);

  return std::unique_ptr<shader>(new shader(vertex_src, fragment_src, bindings));
}

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

shader::shader(const char *vertex_src, const char *fragment_scr, gfx::attribute::binding_pack &binding_pack)
  : handle_(gfx::create_shader(vertex_src, fragment_scr, binding_pack))
{}


