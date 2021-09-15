#pragma once

#include "base/guid.h"
#include "core/meta/type.h"
#include "core/meta/type_info.h"

namespace ecs {

using component_id_t = meta::typeid_t;

template<class T>
struct component_info;

}

#define register_component(__type) \
namespace ecs { \
template<> \
struct component_info<__type> { \
  static component_id_t id() { return meta::get_type(#__type).id(); } \
}; \
}

