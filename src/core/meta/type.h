#pragma once

#include "core/meta/type_info.h"
#include "base/type_name.h"

namespace meta {

class type {
 public:
  explicit type(typeid_t id) : id_(id) {}
  [[nodiscard]] typeid_t id() const { return id_; }
  [[nodiscard]] std::string_view name() const { return details::get_type_info(id_)->name; }
  [[nodiscard]] bool is_valid() const { return details::has_type_info(id_); }

 private:
  typeid_t id_;
};

namespace details {

template<class T>
type_info *get_type_info() {
  static type_info *ptr = details::create_or_get_type_info(type_name<T>());
  return ptr;
}

}

template<class T>
typeid_t get_typeid() { return details::get_type_info<T>()->id; }

inline type get_type(typeid_t id) { return type(id); }

inline type get_type(const std::string_view name) {
  const auto *type_info = details::get_type_info(name);
  return type_info ? type(type_info->id) : type(0);
}

template<class T>
type get_type() {
  const auto *type_info = details::get_type_info<T>();
  return type_info ? type(type_info->id) : type(0);
}

}