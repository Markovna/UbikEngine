#include "os.h"
#include <dlfcn.h>
#include <cstring>
#include <iostream>

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

fs::path find_lib(const char *path, const char *name) {
  std::string filename(to_lib_name(name));

  fs::path filepath(path);
  filepath.append(filename);
  if (fs::exists(filepath))
    return filepath;

  return fs::path();
}

}