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
type_info* get_type_info() {
  static type_info* ptr = details::create_or_get_type_info(type_name<T>());
  return ptr;
}

}

inline type get_type(typeid_t id) { return type(id); }
inline type get_type(const char* name) { return type(details::get_type_info(name)->id); }

template<class T>
type get_type() {
  return type(details::get_type_info<T>()->id);
}

template<class T>
typeid_t get_typeid() { return details::get_type_info<T>()->id; }

namespace details {

template<class T>
interface_info<T>* get_interface_info() {
  static interface_info<T>* inst = details::create_or_get_interface_info<T>(type_name<T>());
  return inst;
}

template<class T>
T* get_interface(interface_info<T>* info, typeid_t id) {
  if (!info) return nullptr;

  if (auto it = info->interfaces.find(id); it != info->interfaces.end()) {
    return (T*) &it->second;
  }

  return nullptr;
}

}

template<class I, class T>
I* get_interface() {
  return details::get_interface(details::get_interface_info<I>(), get_typeid<T>());
}

template<class I>
I* get_interface(typeid_t id) {
  return details::get_interface(details::get_interface_info<I>(), id);
}

}