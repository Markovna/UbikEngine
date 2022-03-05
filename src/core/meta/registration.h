#pragma once

#include "core/meta/type_info.h"
#include "base/type_name.h"
#include "core/meta/type.h"
#include "base/log.h"

#define register_type(__type) meta::registration::type<__type>()

namespace meta::registration {

template<class T>
type type() {
  return get_type(details::add_type(type_name<T>())->id);
}

}