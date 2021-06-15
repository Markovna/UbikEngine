#pragma once

#include <string>

#include "base/guid.h"
#include "core/serialization.h"

struct world;
struct entity;

namespace meta::details {

using to_asset_fn = void (*)(asset&, const void*);
using from_asset_fn = void (*)(const asset&, void*);
using instantiate_fn = void* (*)(world&, const entity&);

struct type_info {
  static const type_info& invalid();

  guid id;
  std::string name;

  to_asset_fn to_asset;
  from_asset_fn from_asset;
  instantiate_fn instantiate;
};

template<class T>
struct typeid_storage {
  static guid id;
};

template<class T>
guid typeid_storage<T>::id = guid::invalid();

template<class T>
const guid& get_typeid() {
  return typeid_storage<T>::id;
}

template<class T>
void set_typeid(guid guid) {
  typeid_storage<T>::id = guid;
}

void add_type(type_info&& info);
const type_info& get_type(guid id);
const type_info& get_type(const char* name);

template<class T>
const type_info& get_type() { return get_type(get_typeid<T>()); }

}