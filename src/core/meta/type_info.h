#pragma once

#include <string>

#include "base/guid.h"
#include "core/assets/asset.h"

struct world;
struct entity;

namespace meta {

using typeid_t = uintptr_t;

namespace details {

using to_asset_fn = void (*)(asset&, const void*);
using from_asset_fn = void (*)(assets::provider*, const asset&, void*);
using instantiate_fn = void* (*)(world&, const entity&);

struct type_info {
  typeid_t id;
  std::string name;

  to_asset_fn to_asset;
  from_asset_fn from_asset;
  instantiate_fn instantiate;
};

inline const type_info* get_type_info(typeid_t id) {
  return (type_info*) id;
}

inline bool has_type_info(typeid_t id) {
  return (type_info*) id;
}

void add_type(type_info&& info);
const type_info* get_type_info(std::string_view);

}

}