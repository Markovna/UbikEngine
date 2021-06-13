#pragma once

#include "core/world.h"
#include "core/serialization.h"
#include "core/meta/type_info.h"
#include "core/meta/schema.h"
#include "base/log.h"

#define register_type(__type) meta::registration::type<__type>(#__type)

namespace meta {

namespace details {

template<typename T>
using load_detector
  = decltype(std::declval<serialization<T>>().from_asset(std::declval<const asset&>(), std::declval<T*>()));

template<typename T>
using save_detector
  = decltype(std::declval<serialization<T>>().to_asset(std::declval<asset &>(), std::declval<const T*>()));

template<typename, typename = void>
struct is_serializable : std::false_type {};

template<typename T>
struct is_serializable<T, std::void_t<save_detector<T>, load_detector<T>>> : std::true_type {};

template<class Component>
static void save_impl(::asset& asset, const void *ptr) {
  if constexpr (is_serializable<Component>::value) {
    ::asset& comp_asset = asset.emplace_back();
    comp_asset["__type"] = ::meta::details::get_type<Component>().name;
    comp_asset["__guid"] = guid::generate();
    serialization<Component>{}.to_asset(comp_asset, static_cast<const Component*>(ptr));
  }
}

template<class Component>
static void load_impl(const asset &asset, void *ptr) {
  if constexpr (is_serializable<Component>::value) {
    serialization<Component>{}.from_asset(asset, static_cast<Component*>(ptr));
  }
}

template<class Component>
static void* instantiate_impl(world* w, const entity& e) {
  if constexpr (std::is_base_of<component_base, Component>::value) {
    return static_cast<void*>(&w->add_component<Component>(e));
  } else {
    logger::core::Error("Couldn't instantiate type {}: it's not a component.", ::meta::details::get_type<Component>().name);
  }
}

}

namespace registration {

template<class T>
void type(const char* name) {
  guid id = get_schema(name).guid;
  details::set_typeid<T>(id);
  details::add_type({
      .id = id,
      .name = name,
      .save_ptr = details::save_impl<T>,
      .load_ptr = details::load_impl<T>,
      .instantiate_ptr = details::instantiate_impl<T>
    });
}

}

}