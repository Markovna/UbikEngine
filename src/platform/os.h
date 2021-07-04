#pragma once

#include "platform/file_system.h"
#include <string>

namespace os {

fs::path find_lib(const char* path, const char* name);

void* load_lib(const char*);

int unload_lib(void*);

void* get_symbol(void*, const char*);

int64_t get_timestamp(const fs::path&);

};


