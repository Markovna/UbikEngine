#pragma once

#include "base/math.h"
#include "base/slot_map.h"
#include "core/ecs.h"

#include <vector>
#include <unordered_map>

using entity_id = ecs::entity;

struct entity {
  entity_id id = ecs::invalid;

  [[nodiscard]] bool is_valid() const { return id != ecs::invalid; }

  static const entity& invalid() {
    static const entity invalid {};
    return invalid;
  }

  bool operator==(entity other) const { return id == other.id; }
  bool operator!=(entity other) const { return !(*this == other); }

  explicit operator bool() const { return is_valid(); }
};

struct link_component {
  entity parent;
  entity next;
  entity prev;
  entity child;
  entity child_last;
  uint32_t children_size = 0;
};

struct transform_component {
  transform local;
  mutable transform world;
  mutable bool dirty = false;
};

namespace ecs {

  class system {
   public:
    virtual ~system() = default;
  };

}

class world : public ecs::registry {
 private:
  using system_update_f = void(*)(void*, world*);

  struct system_info {
    meta::typeid_t id;
    std::unique_ptr<ecs::system> ptr;
    system_update_f update_ptr;
  };

  template<class ...>
  struct types {
    using type = types;
  };

  template<class>
  struct args;

  template<class T, class R, class ...Args>
  struct args<R (T::*)(Args...)> : public types<Args...> { };

  template<class T>
  using args_t = typename args<T>::type;

  template<class>
  struct update_invoker;

  template<class ...Components>
  struct update_invoker<types<world*, ecs::component_view<Components...>>> {
    template<class T>
    static void invoke(void* ptr, world* w) {
      T& ref = *static_cast<T*>(ptr);
      ref.update(w, w->view<Components...>());
    }
  };

 public:
  static std::unique_ptr<world> create() { return std::make_unique<world>(); }

  world();

  entity create_entity(const transform& local = {}, entity parent = entity::invalid(), entity next = entity::invalid());
  void destroy_entity(entity entity);

  entity load_from_asset(const asset& asset, entity parent = entity::invalid(), entity next = entity::invalid());

  void set_parent(entity ent, entity parent, entity next = entity::invalid());

  [[nodiscard]] entity parent(entity ent) const;

  [[nodiscard]] transform local_transform(entity entity) const;
  [[nodiscard]] vec3 local_position(entity entity) const;
  [[nodiscard]] quat local_rotation(entity entity) const;

  void set_local_transform(entity entity, const transform& local);
  void set_local_position(entity entity, const vec3& pos);
  void set_local_rotation(entity entity, const quat& rot);

  transform world_transform(entity entity) const;
  void set_world_transform(entity ent, const transform& world);

  [[nodiscard]] entity root() const { return ecs::registry::get<link_component>(root_.id).child; }
  [[nodiscard]] size_t roots_size() const { return ecs::registry::get<link_component>(root_.id).children_size; }

  [[nodiscard]] entity child(entity entity) const {
    return ecs::registry::get<link_component>(entity.id).child;
  }

  [[nodiscard]] bool is_child_of(entity ent, entity parent_candidate) const;

  [[nodiscard]] entity prev(entity entity) const {
    return ecs::registry::get<link_component>(entity.id).prev;
  }

  [[nodiscard]] entity next(entity entity) const {
    return ecs::registry::get<link_component>(entity.id).next;
  }

  [[nodiscard]] size_t children_size(entity entity) const {
    return ecs::registry::get<link_component>(entity.id).children_size;
  }

  template<class T, class ...Args>
  void register_system(Args&&... args) {
    meta::typeid_t id = meta::get_typeid<T>();
    systems_index_[id] = systems_.size();
    system_info& info = systems_.emplace_back();
    info.id = id;
    info.ptr = std::make_unique<T>(std::forward<Args>(args)...);
    info.update_ptr = &update_invoker<args_t<decltype(&T::update)>>::template invoke<T>;
  }

  template<class T>
  void remove_system() {
    meta::typeid_t id = meta::get_typeid<T>();
    if (auto it = systems_index_.find(id); it != systems_index_.end()) {
      uint32_t index = it->second;
      std::swap(systems_[index], systems_.back());
      systems_index_[systems_[index].id] = index;
      systems_index_.erase(id);
      systems_.pop_back();
    }
  }

  void update_systems() {
    for (auto& system : systems_) {
      system.update_ptr(system.ptr.get(), this);
    }
  }

 private:
  void set_parent_impl(entity ent, entity parent, entity next);
  [[nodiscard]] const transform& resolve_transform(entity ent) const;
  void set_transform_dirty(entity ent, bool dirty) const;

 private:
  entity root_;
  std::vector<system_info> systems_;
  std::unordered_map<meta::typeid_t, uint32_t> systems_index_;
};

template<class T>
void instantiate_component(world& world, entity& e) {
  world.emplace<T>(e.id);
}
