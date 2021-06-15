#pragma once

#include "base/guid.h"

#include <string>

namespace meta {

struct schema_info {
  guid guid;
  std::string name;
};

void load_schemas(const char* path);

guid get_schema_id(const char *name);
const schema_info& get_schema(const char* name);

}