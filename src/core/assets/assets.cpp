#include "assets.h"
#include "core/assets/texture.h"
#include "core/assets/shader.h"

#include <fstream>

namespace assets {

namespace details {

context* g_context;

context* get_context() { return g_context; }

bool read_file(const std::filesystem::path &path, std::ostream &stream) {
  std::string content;
  std::ifstream file;
  file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
  try {
    file.open(path, std::ios::in | std::ios::binary);
    stream << file.rdbuf();
    file.close();
    return true;
  }
  catch (std::ifstream::failure &e) {
    logger::core::Error("Couldn't read file {0} {1}", path.string(), e.what());
  }
  return false;
}

template<>
registry<texture>* get_registry() {
  return g_context->texture_registry.get();
}

template<>
registry<shader>* get_registry() {
  return g_context->shader_registry.get();
}

}

void init(const char *project_path) {
  details::g_context = new details::context();
  details::g_context->project_path = project_path;
  details::g_context->texture_registry = std::make_unique<details::registry<texture>>();
  details::g_context->shader_registry = std::make_unique<details::registry<shader>>();
}

void shutdown() {
  delete details::g_context;
}

}