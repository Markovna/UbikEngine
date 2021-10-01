#pragma once

#include <string>

#include "base/guid.h"
#include "core/assets/asset.h"

namespace meta {

using typeid_t = uintptr_t;

namespace details {

using to_asset_fn = void (*)(asset&, const void*);
using from_asset_fn = void (*)(assets::provider*, const asset&, void*);

struct interface_info_base {
  typeid_t id;
  std::string name;
};

template<class T>
struct interface_info : public interface_info_base {
  std::unordered_map<typeid_t, T> interfaces;
};

struct type_info {
  typeid_t id;
  std::string name;
};

class registry {
 public:
//  type_info* add_type(type_info&& info) {
//    type_info* ptr;
//    if (auto it = types_name_index_.find(info.name); it != types_name_index_.end()) {
//      ptr = (type_info*) it->second;
//      *ptr = std::move(info);
//    } else {
//      auto& item = types_.emplace_back(std::make_unique<type_info>(std::move(info)));
//      ptr = item.get();
//    }
//    ptr->id = (uintptr_t) ptr;
//    types_name_index_[ptr->name] = ptr->id;
//    return ptr;
//  }

  [[nodiscard]] const type_info* get(std::string_view name) const {
    if (auto it = types_name_index_.find(std::string { name }); it != types_name_index_.end()) {
      return (type_info*) it->second;
    }

    return nullptr;
  }

  type_info* create_or_get_type(std::string_view name) {
    if (auto it = types_name_index_.find(std::string { name }); it != types_name_index_.end()) {
      return (type_info*) it->second;
    }

    auto& item = types_.emplace_back(std::make_unique<type_info>());
    item->name = name;
    item->id = (uintptr_t) item.get();
    types_name_index_[item->name] = item->id;
    return item.get();
  }

  template<class T>
  interface_info<T>* create_or_get_interface(std::string_view name) {
    if (auto it = interfaces_name_index_.find(std::string { name }); it != interfaces_name_index_.end()) {
      return (interface_info<T>*) it->second;
    }

    auto& item = interfaces_.emplace_back(std::make_unique<interface_info<T>>());
    item->name = name;
    item->id = (uintptr_t) item.get();
    interfaces_name_index_[item->name] = item->id;
    return static_cast<interface_info<T>*>(item.get());
  }

 private:
  std::vector<std::unique_ptr<type_info>> types_;
  std::vector<std::unique_ptr<interface_info_base>> interfaces_;
  std::unordered_map<std::string, typeid_t> types_name_index_;
  std::unordered_map<std::string, typeid_t> interfaces_name_index_;
};

registry& get_registry();

inline const type_info* get_type_info(typeid_t id) {
  return (type_info*) id;
}

inline bool has_type_info(typeid_t id) {
  return (type_info*) id;
}

void add_type(std::string_view name);
const type_info* get_type_info(std::string_view);
type_info* create_or_get_type_info(std::string_view);

template<class T>
interface_info<T>* create_or_get_interface_info(std::string_view name) {
  return get_registry().create_or_get_interface<T>(name);
}

template<class I>
void add_interface(std::string_view i_name, typeid_t id, const I& i_instance) {
  details::interface_info<I>* info = details::create_or_get_interface_info<I>(i_name);
  info->interfaces[id] = i_instance;
}

}

}