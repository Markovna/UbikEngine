#pragma once

#include <string>

#include "base/guid.h"
#include "core/serialization.h"

struct world;
struct entity;

namespace meta::details {

using save_ptr_t = void (*)(asset&, const void*);
using load_ptr_t = void (*)(const asset&, void*);
using instantiate_ptr_t = void* (*)(world*, const entity&);

struct type_info {
  static const type_info& invalid();

  guid id;
  std::string name;
  save_ptr_t save_ptr;
  load_ptr_t load_ptr;
  instantiate_ptr_t instantiate_ptr;
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