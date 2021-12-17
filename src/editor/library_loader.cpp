#include "library_loader.h"

library_loader::library_loader()
  : temp_folder_(fs::append(fs::temp_directory_path(), "libs_tmp").c_str())
{
  if (!fs::exists(temp_folder_))
    fs::create_directory(temp_folder_);
}

library_loader::~library_loader() {
  reset();
}

void library_loader::reset() {
  for (auto& [_, info] : libs_) {
    os::unload_lib(info.symbols);
    fs::remove(info.temp_path);
  }

  libs_.clear();
}

fs::path library_loader::copy_to_temp(const char *name, const fs::path &src_path, uint32_t version) {
  fs::path path {temp_folder_};
  path.append(name);
  path.concat(std::to_string(version));
  path.replace_extension(src_path.extension());

  std::ifstream src_stream(src_path, std::ios::binary);
  std::ofstream dst_stream(path, std::ios::binary);
  dst_stream << src_stream.rdbuf();
  return path;
}
