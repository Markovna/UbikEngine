#pragma once

#include "type.h"
#include "base/slot_map.h"
#include "core/ecs.h"

class interface_registry {
 public:
  template<class I>
  void register_interface(meta::typeid_t type_id, I interface) {
    auto it = type_to_entity_.find(type_id);
    if (it == type_to_entity_.end()) {
      auto [it0, _] = type_to_entity_.emplace(std::make_pair(type_id, registry_.create()));
      it = it0;
    }

    registry_.emplace<interface_info<I>>(it->second, interface_info<I> { .id = type_id, .interface = std::move(interface)});
  }

  template<class I>
  [[nodiscard]] const I* get_interface(meta::typeid_t id) const {
    auto it = type_to_entity_.find(id);
    if (it == type_to_entity_.end())
      return nullptr;

    if (auto* info = registry_.try_get<interface_info<I>>(it->second))
      return &info->interface;

    return nullptr;
  }

  template<class I>
  bool has_interface() {
    return registry_.has<interface_info<I>>();
  }

  template<class I>
  auto get_interface_view() {
    return registry_.view<interface_info<I>>();
  }

 private:
  template<class T>
  struct interface_info {
    meta::typeid_t id;
    T interface;
  };

  ecs::registry registry_;
  std::unordered_map<meta::typeid_t, ecs::entity> type_to_entity_;
};