#include "type_info.h"

#include "base/log.h"

namespace meta::details {

registry& get_registry() {
  static registry inst;
  return inst;
}

type_info* add_type(std::string_view name) {
  return get_registry().create_or_get_type(name);
}

const type_info* get_type_info(std::string_view name) {
  return get_registry().get(name);
}

type_info* create_or_get_type_info(std::string_view name) {
  return get_registry().create_or_get_type(name);
}

type_info* registry::create_or_get_type(std::string_view name) {
  if (auto it = types_name_index_.find(std::string { name }); it != types_name_index_.end()) {
    return (type_info*) it->second;
  }

  auto& item = types_.emplace_back(std::make_unique<type_info>());
  item->name = name;
  item->id = (uintptr_t) item.get();
  types_name_index_[item->name] = item->id;
  logger::core::Info("Register type {}", name, types_name_index_.size());
  return item.get();
}

}