#pragma once

#include "base/json.hpp"
#include "base/detector.h"
#include "base/log.h"
#include "platform/file_system.h"

#include <fstream>
#include <base/guid.h>

using asset = nlohmann::json;

template<class T>
using from_json_function = decltype(std::declval<nlohmann::json>().get<T>());

template<class T>
using to_json_function = decltype(nlohmann::json { std::declval<T>() });

template<class T, class = void>
struct has_from_json : std::false_type {};

template<class T, class = void>
struct has_to_json : std::false_type {};

template<class T>
struct has_from_json<T, void> {
  static constexpr bool value = is_detected_exact<T, from_json_function, std::decay_t<T>>::value;
};

template<class T>
struct has_to_json<T, void> {
  static constexpr bool value = is_detected_exact<nlohmann::json, to_json_function, std::decay_t<T>>::value;
};

namespace assets {

struct provider;

asset read(const char* path);
asset read(const fs::path& path);

void write(const asset&, const char* path);
void write(const asset&, const fs::path& path);

template<class T, class = std::enable_if_t<has_from_json<T>::value>>
void get(provider*, const asset& asset, const char* key, T& value) {
  value = asset.at(key);
}

template <class T, class = std::enable_if_t<has_to_json<T>::value>>
void set(asset& asset, const char* key, T&& value) {
  asset.at(key) = std::forward<T>(value);
}

}