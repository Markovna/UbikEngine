#pragma once

#include "core/assets/asset.h"
#include "base/detector.h"

template <class T>
struct serializer;

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
  static constexpr bool value = is_detected_exact<void, from_asset_function, clear_type, const asset&, clear_type&>::value;
};

template<class T>
struct has_to_asset<T, void> {
  using clear_type = std::decay_t<T>;
  static constexpr bool value = is_detected_exact<void, to_asset_function, clear_type, asset &, const clear_type&>::value;
};

namespace assets {

template<class T, class = std::enable_if_t<!has_from_json<T>::value>, class = std::enable_if_t<has_from_asset<T>::value>>
void get(const asset& asset, const char* key, T& value) {
  serializer<std::decay_t<T>>::from_asset(asset.at(key), value);
}

template <class T, class = std::enable_if_t<!has_to_json<T>::value>, class = std::enable_if_t<has_to_asset<T>::value>>
void set(asset& asset, const char* key, T&& value) {
  serializer<std::decay_t<T>>::to_asset(asset[key], std::forward<T>(value));
}

}