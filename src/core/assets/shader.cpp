#include "shader.h"

#include <sstream>

template<>
std::unique_ptr<shader> assets::loader::load(std::istream& stream) {
  gfx::attribute::binding_pack bindings;
  size_t vertex_size, fragment_size;

  stream.read((char*) &bindings, sizeof(bindings));

  stream.read((char*) &vertex_size, sizeof(vertex_size));
  char vertex_src[vertex_size];
  stream.read(vertex_src, vertex_size);

  stream.read((char*) &fragment_size, sizeof(fragment_size));
  char fragment_src[fragment_size];
  stream.read(fragment_src, fragment_size);

  return std::unique_ptr<shader> { new shader(vertex_src, fragment_src, bindings) };
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

namespace assets::compiler {

template<>
bool compile<shader>(std::ifstream& stream, const asset& meta, std::ostream& output) {
  std::string source { std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>() };

  static const std::string type_token = "#type";
  static const std::string vertex_type = "vertex";
  static const std::string fragment_type = "fragment";

  std::string vertex_shader, fragment_shader;
  gfx::attribute::binding_pack bindings;

  size_t pos = source.find(type_token, 0);
  std::string type;
  while (pos != std::string::npos) {
    size_t eol = source.find_first_of('\n', pos);
    size_t begin = pos + type_token.size() + 1;
    type = source.substr(begin, eol - begin);
    uint64_t nextLinePos = eol + 1;
    pos = source.find(type_token, nextLinePos);

    if (type == vertex_type) {
      vertex_shader = source.substr(nextLinePos, pos == std::string::npos ? pos : pos - nextLinePos);
    } else if (type == fragment_type) {
      fragment_shader = source.substr(nextLinePos, pos == std::string::npos ? pos : pos - nextLinePos);
    }
  }

  {
    static const std::string token = "#binding";
    size_t pos = vertex_shader.find(token, 0);
    int loc = 0;
    const char* location_template_str = "layout (location = #)";
    const size_t location_str_pos = 19;
    while (pos != std::string::npos) {
      size_t eol = vertex_shader.find_first_of('\n', pos);
      size_t begin = pos + token.size() + 1;
      std::string binding_type = vertex_shader.substr(begin, eol - begin);

      std::string location_str(location_template_str);
      location_str.replace(location_str_pos, 1, std::to_string(loc));
      vertex_shader.replace(pos, eol - pos, location_str);

      gfx::attribute::binding::type binding;
      if (gfx::attribute::binding::try_parse(binding_type, binding)) {
        gfx::attribute::set_pack(bindings, binding, loc);
      } else {
        logger::core::Error("Can't parse {}", binding_type);
      }

      pos = vertex_shader.find(token, pos);
      loc++;
    }
  }

  size_t vertex_size = vertex_shader.size() + 1;
  size_t fragment_size = fragment_shader.size() + 1;
  output.write((char*) &bindings, sizeof(bindings));

  output.write((char*) &vertex_size, sizeof(vertex_size));
  output.write(vertex_shader.c_str(), vertex_size);

  output.write((char*) &fragment_size, sizeof(fragment_size));
  output.write(fragment_shader.c_str(), fragment_size);
  return true;
}

}

