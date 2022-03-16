#include "file_system.h"
#include "base/log.h"

#include <fstream>

namespace fs {

path concat(const path& lhs, const char* rhs) {
  path result {lhs};
  result.concat(rhs);
  return result;
}

path concat(const path& lhs, const path& rhs) {
  return concat(lhs, rhs.c_str());
}

path append(const path& lhs, const char* rhs) {
  path result {lhs};
  result.append(rhs);
  return result;
}

path append(const path& lhs, const path& rhs) {
  return append(lhs, rhs.c_str());
}

path meta(const char* _path) {
  path meta_path { _path };
  meta_path.append(".meta");
  return meta_path;
}

path meta(const path& _path) {
  return meta(_path.c_str());
}

std::ifstream read_file(const char* path, std::ios::openmode mode)  {
  std::ifstream stream;
  stream.exceptions(std::ifstream::failbit | std::ifstream::badbit);
  try {
    stream.open(path, mode);
  }
  catch (std::ifstream::failure &e) {
    logger::core::Error("Couldn't read file {0} {1}", path, e.what());
  }
  return stream;
}

std::ifstream read_file(const path &path, std::ios::openmode mode) {
  return read_file(path.c_str(), mode);
}

void assure(const path& path) {
  if (!fs::exists(path)) {
    fs::create_directory(path);
  }
}

static path project_path_;

path project_path() { return project_path_; }
void project_path(const fs::path& path) {
  project_path_ = path;
}

path to_project_path(const path &p) {
  return p.is_absolute() ? p : fs::append(fs::project_path_, p);
}

}