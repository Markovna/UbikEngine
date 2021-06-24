#include "asset.h"

namespace assets {

asset read(const fs::path& path) {
  std::ifstream file = fs::read_file(path, std::ios::in);
  asset data = asset::parse(file);
  return data;
}

asset read(const char *path) {
  return read(fs::path {path});
}

void write(const asset& asset, const char* path) {
  std::ofstream dst_stream(path);
  dst_stream << asset.dump(2);
}

void write(const asset& asset, const fs::path& path) {
  write(asset, path.c_str());
}

}