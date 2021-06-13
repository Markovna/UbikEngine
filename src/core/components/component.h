#pragma once

#include "base/guid.h"
#include "core/meta/type.h"

struct component_base {
  using id_t = meta::typeid_t;
};

template<class T>
struct component : component_base {
  static id_t id() { return meta::get_typeid<T>(); }
};

