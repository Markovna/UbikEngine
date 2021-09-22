#pragma once

#include "core/meta/type_info.h"
#include "base/type_name.h"

//struct world;
//struct entity;

namespace meta {

class type {
 public:
  explicit type(typeid_t id) : id_(id) {}
  [[nodiscard]] typeid_t id() const { return id_; }
  [[nodiscard]] std::string_view name() const { return details::get_type_info(id_)->name; }
  [[nodiscard]] bool is_valid() const { return details::has_type_info(id_); }

  void* instantiate(world& w, const entity& e) const {
    return details::get_type_info(id_)->instantiate(w, e);
  }

  void to_asset(asset& asset, const void* obj) const {
    details::get_type_info(id_)->to_asset(asset, obj);
  }

  void from_asset(assets::provider* p, const asset& asset, void* obj) const {
    details::get_type_info(id_)->from_asset(p, asset, obj);
  }

 private:
  typeid_t id_;
};

inline type get_type(typeid_t id) { return type(id); }
inline type get_type(const char* name) { return type(details::get_type_info(name)->id); }

namespace details {

template<class T>
type& get_type() {
  static type t { details::get_type_info(type_name<T>())->id };
  return t;
}

// TODO
//template<class T>
//void init(type &t) {
//  get_type<T>() = type { details::get_type_info(type_name<T>())->id };
//}

}

template<class T>
type get_type() {
  return details::get_type<T>();
}

template<class T>
typeid_t get_typeid() { return get_type<T>().id(); }

}