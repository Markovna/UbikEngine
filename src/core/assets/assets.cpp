#include "assets.h"

#include <fstream>

namespace assets {

namespace details {

context* get_context() {
  static context inst;
  return &inst;
}

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

}

void init(const char *project_path) {
  details::get_context()->project_path = project_path;
}

void shutdown() {}

}