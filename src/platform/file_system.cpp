#include "file_system.h"
#include "base/log.h"

#include <fstream>

namespace fs {

path absolute(const char* _path) {
  path result { paths::project() };
  result.append(_path);
  return result;
}

path absolute(const path& _path) {
  return absolute(_path.c_str());
}

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

void paths::project(const path& _path) {
  details::get_config().project_path = _path;

  path ubik_path { append(_path, ".ubik") };
  if (!exists(ubik_path)) {
    fs::create_directory(ubik_path);
  }
  details::get_config().cache_path = ubik_path;
}

const path &paths::project() { return ::fs::details::get_config().project_path; }
const path &paths::cache() { return ::fs::details::get_config().cache_path; }

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

details::config &details::get_config() {
  static details::config inst;
  return inst;
}

}