#pragma once

#include "base/guid.h"
#include "core/meta/type_info.h"

struct world;
struct entity;

namespace meta {

using typeid_t = guid;

class type {
 public:
  explicit type(typeid_t id) : id_(id) {}
  [[nodiscard]] typeid_t id() const { return id_; }
  [[nodiscard]] const char* name() const { return details::get_type(id_).name.c_str(); }

  void* instantiate(world& w, const entity& e) const {
    return details::get_type(id_).instantiate(w, e);
  }

  void to_asset(asset& asset, const void* obj) const {
    details::get_type(id_).to_asset(asset, obj);
  }

  void from_asset(const asset& asset, void* obj) const {
    details::get_type(id_).from_asset(asset, obj);
  }

 private:
  typeid_t id_;
};

template<class T>
typeid_t get_typeid() { return details::get_typeid<T>(); }

inline type get_type(typeid_t id) { return type(id); }
inline type get_type(const char* name) { return type(details::get_type(name).id); }

template<class T>
type get_type() { return get_type(get_typeid<T>()); }

}