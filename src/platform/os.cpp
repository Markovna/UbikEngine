#include "os.h"
#include <dlfcn.h>
#include <cstring>
#include <iostream>

#if defined(_WIN32)
#define UBIK_WINDOWS

#elif defined(__linux__)
#define UBIK_LINUX

#elif defined(__APPLE__)
#define UBIK_OSX

#else
#error "Unknown/unsupported platform"
#endif // UBIK_WINDOWS || UBIK_LINUX || UBIK_OSX

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

#if defined(UBIK_LINUX) || defined(UBIK_OSX)
#include <sys/stat.h>

int64_t get_timestamp(const fs::path& path) {
  struct stat stats;
  if (stat(path.c_str(), &stats) == -1) {
    return -1;
  }
  if (stats.st_size == 0) {
    return -1;
  }
#if defined(UBIK_OSX)
  return stats.st_mtime;
#else
  return stats.st_mtim.tv_sec;
#endif

}

#endif



}