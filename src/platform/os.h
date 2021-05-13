#pragma once

#include <string>
#include <filesystem>

namespace os {
  std::filesystem::path find_lib(const char* path, const char* name);
  void* load_lib(const char*);
  int unload_lib(void*);
  void* get_symbol(void*, const char*);
};


