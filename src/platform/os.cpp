#include "os.h"
#include <dlfcn.h>
#include <cstring>
#include <iostream>
#include <filesystem>

namespace os {

void* load_lib(const char* path) {
  // TODO
  if (dlopen(path, RTLD_LOCAL | RTLD_NOLOAD)) {
    std::cerr << "Lib " << path << " has been already loaded" << std::endl;
  }

  return dlopen(path, RTLD_LOCAL);
}

void* get_symbol(void* handle, const char* name) {
  return dlsym(handle, name);
}

int unload_lib(void* handle) {
  return dlclose(handle);
}

std::string to_lib_name(const char* name) {
  return "lib" + std::string(name) + ".dylib";
}

std::filesystem::path find_lib(const char *path, const char *name) {
  std::string filename(to_lib_name(name));

  std::filesystem::path filepath(path);
  filepath.append(filename);
  if (std::filesystem::exists(filepath))
    return filepath;

  return std::filesystem::path();
}

}