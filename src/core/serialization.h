#pragma once

#include "base/json.hpp"
#include "base/detected.h"

using asset = nlohmann::json;

template <class T>
struct serialization;

template<class T, typename... Args>
using from_asset_function = decltype(serialization<T>::from_asset(std::declval<Args>()...));

template<class T, typename... Args>
using to_asset_function = decltype(serialization<T>::to_asset(std::declval<Args>()...));

template<class T, class = void>
struct has_from_asset : std::false_type {};

template<class T>
struct has_from_asset<T, void> {
  using clear_type = std::decay_t<T>;
  static constexpr bool value = is_detected_exact<void, from_asset_function, clear_type, const asset&, clear_type&>::value;
};

template<class T, class = void>
struct has_to_asset : std::false_type {};

template<class T>
struct has_to_asset<T, void> {
  using clear_type = std::decay_t<T>;
  static constexpr bool value = is_detected_exact<void, to_asset_function, clear_type, asset &, const clear_type&>::value;
};

namespace assets {

template<class T>
using from_json_function = decltype(std::declval<nlohmann::json>().get<T>());

template<class T, class = void>
struct has_from_json : std::false_type {};

template<class T>
struct has_from_json<T, void> {
  static constexpr bool value = is_detected_exact<T, from_json_function, std::decay_t<T>>::value;
};

template<class T>
using to_json_function = decltype(nlohmann::json { std::declval<T>() });

template<class T, class = void>
struct has_to_json : std::false_type {};

template<class T>
struct has_to_json<T, void> {
  static constexpr bool value = is_detected_exact<nlohmann::json, to_json_function, std::decay_t<T>>::value;
};


template<class T, class = std::enable_if_t<has_from_json<T>::value>>
void get(const asset& asset, const char* key, T& value) {
  value = asset.at(key);
}

template<class T, class = std::enable_if_t<!has_from_json<T>::value>, class = std::enable_if_t<has_from_asset<T>::value>>
void get(const asset& asset, const char* key, T& value) {
  serialization<std::decay_t<T>>::from_asset(asset.at(key), value);
}

template <class T, class = std::enable_if_t<has_to_json<T>::value>>
void set(asset& asset, const char* key, T&& value) {
  asset[key] = std::forward<T>(value);
}

template <class T, class = std::enable_if_t<!has_to_json<T>::value>, class = std::enable_if_t<has_to_asset<T>::value>>
void set(asset& asset, const char* key, T&& value) {
  serialization<std::decay_t<T>>::to_asset(asset[key], std::forward<T>(value));
}



}