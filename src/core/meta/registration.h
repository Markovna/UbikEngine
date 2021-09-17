#pragma once

#include "core/world.h"
#include "core/serialization.h"
#include "core/meta/type_info.h"
#include "core/meta/schema.h"
#include "base/log.h"

#define register_type(__type) meta::registration::type<__type>(#__type)

namespace meta {

namespace details {

template<class, class = void>
struct has_component_info: std::false_type {};

template<class T>
struct has_component_info<T, std::void_t<decltype(::ecs::component_info<T>::id())>> : std::true_type {};

template<class T>
inline constexpr bool has_component_info_v = has_component_info<T>::value;

template<class Component>
static void to_asset_impl(::asset& asset, const void *ptr) {
  if constexpr (assets::has_to_asset<Component>::value) {
    ::asset& comp_asset = asset.emplace_back();
    serializer<Component>::to_asset(comp_asset, *static_cast<const Component*>(ptr));
    if constexpr (has_component_info_v<Component>) {
      comp_asset["__type"] = ::meta::get_type(::ecs::component_info<Component>::id()).name();
    }
    comp_asset["__guid"] = guid::generate();
  }
}

template<class Component>
static void from_asset_impl(assets::provider* p, const asset &asset, void *ptr) {
  if constexpr (assets::has_from_asset<Component>::value) {
    serializer<Component>::from_asset(p, asset, *static_cast<Component*>(ptr));
  }
}

template<class Component>
static void* instantiate_impl(world& w, const entity& e) {
  if constexpr (has_component_info_v<Component>) {
    return static_cast<void*>(&w.add_component<Component>(e));
  } else {
    logger::core::Error("Couldn't instantiate type: it's not a component.");
    return nullptr;
  }
}

}

namespace registration {

template<class T>
void type(const char* name) {
  guid id = get_schema_id(name);
  if (id.is_valid()) {
    logger::core::Info("Register type {}. GUID: {}", name, id.str());
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

}