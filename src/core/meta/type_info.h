#pragma once

#include <string>

#include "base/guid.h"
#include "core/assets/asset.h"

struct world;
struct entity;

namespace meta::details {

using to_asset_fn = void (*)(asset&, const void*);
using from_asset_fn = void (*)(assets::provider*, const asset&, void*);
using instantiate_fn = void* (*)(world&, const entity&);

struct type_info {
  static const type_info& invalid();

  guid id;
  std::string name;

  to_asset_fn to_asset;
  from_asset_fn from_asset;
  instantiate_fn instantiate;
};

void add_type(type_info&& info);
const type_info& get_type(guid id);
const type_info& get_type(const char* name);

}