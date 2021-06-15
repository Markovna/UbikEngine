#pragma once

#include "core/world.h"
#include "core/serialization.h"
#include "core/meta/type_info.h"
#include "core/meta/schema.h"
#include "base/log.h"

#define register_type(__type) meta::registration::type<__type>(#__type)

namespace meta {

namespace details {

template<class T>
using is_component = std::is_base_of<component_base, T>;

template<class Component>
static void to_asset_impl(::asset& asset, const void *ptr) {
  if constexpr (has_to_asset<Component>::value) {
    ::asset& comp_asset = asset.emplace_back();
    serialization<Component>::to_asset(comp_asset, *static_cast<const Component*>(ptr));
    comp_asset["__type"] = ::meta::details::get_type<Component>().name;
    comp_asset["__guid"] = guid::generate();
  }
}

template<class Component>
static void from_asset_impl(const asset &asset, void *ptr) {
  if constexpr (has_from_asset<Component>::value) {
    serialization<Component>::from_asset(asset, *static_cast<Component*>(ptr));
  }
}

template<class Component>
static void* instantiate_impl(world& w, const entity& e) {
  if constexpr (is_component<Component>::value) {
    return static_cast<void*>(&w.add_component<Component>(e));
  } else {
    logger::core::Error("Couldn't instantiate type {}: it's not a component.", ::meta::details::get_type<Component>().name);
    return nullptr;
  }
}

}

namespace registration {

template<class T>
void type(const char* name) {
  guid id = get_schema_id(name);
  details::set_typeid<T>(id);
  details::add_type({
      .id = id,
      .name = name,
      .to_asset = details::to_asset_impl<T>,
      .from_asset = details::from_asset_impl<T>,
      .instantiate = details::instantiate_impl<T>
  });
}

}

}