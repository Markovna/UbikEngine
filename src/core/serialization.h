#pragma once

#include "core/assets/asset.h"
#include "base/detector.h"
#include "core/meta/type.h"

#define register_serializer_i(__type) \
meta::registration::interface<__type>(serializer_i { \
  .to_asset = assets::to_asset_impl<__type>, \
  .from_asset = assets::from_asset_impl<__type> \
});

template <class T>
struct serializer;

struct serializer_i {
  void (*to_asset)(asset&, const void*);
  void (*from_asset)(assets::provider*, const asset&, void*);
};

namespace assets {

struct provider;

template<class T, typename... Args>
using from_asset_function = decltype(serializer<T>::from_asset(std::declval<Args>()...));

template<class T, typename... Args>
using to_asset_function = decltype(serializer<T>::to_asset(std::declval<Args>()...));

template<class T, class = void>
struct has_from_asset : std::false_type {};

template<class T, class = void>
struct has_to_asset : std::false_type {};

template<class T>
struct has_from_asset<T, void> {
  using clear_type = std::decay_t<T>;
  static constexpr bool value = is_detected_exact<void, from_asset_function, clear_type, assets::provider*, const asset&, clear_type&>::value;
};

template<class T>
struct has_to_asset<T, void> {
  using clear_type = std::decay_t<T>;
  static constexpr bool value = is_detected_exact<void, to_asset_function, clear_type, asset &, const clear_type&>::value;
};

template<class T>
static void to_asset_impl(::asset& asset, const void *ptr) {
  if constexpr (has_to_asset<T>::value) {
    ::asset& comp_asset = asset.emplace_back();
    serializer<T>::to_asset(comp_asset, *static_cast<const T*>(ptr));
    comp_asset["__type"] = meta::get_type<T>().name();
    comp_asset["__guid"] = guid::generate();
  }
}

template<class T>
static void from_asset_impl(assets::provider* p, const asset &asset, void *ptr) {
  if constexpr (has_from_asset<T>::value) {
    serializer<T>::from_asset(p, asset, *static_cast<T*>(ptr));
  }
}

template<class T, class = std::enable_if_t<!has_from_json<T>::value>, class = std::enable_if_t<has_from_asset<T>::value>>
void get(assets::provider* p, const asset& asset, const char* key, T& value) {
  serializer<std::decay_t<T>>::from_asset(p, asset.at(key), value);
}

template <class T, class = std::enable_if_t<!has_to_json<T>::value>, class = std::enable_if_t<has_to_asset<T>::value>>
void set(asset& asset, const char* key, T&& value) {
  serializer<std::decay_t<T>>::to_asset(asset.at(key), std::forward<T>(value));
}


}