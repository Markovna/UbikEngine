#pragma once

#include "core/world.h"
#include "core/serialization.h"
#include "core/meta/type_info.h"
#include "core/meta/schema.h"
#include "base/log.h"

#define register_type(__type) meta::registration::type<__type>()

namespace meta {

namespace registration {

template<class T>
void type() {
  details::add_type(type_name<T>());
}

template<class T, class I>
void interface(const I& instance) {
  logger::core::Info("Register interface {} for type {}", type_name<I>(), type_name<T>());
  details::add_interface(type_name<I>(), get_typeid<T>(), instance);
}



}

}