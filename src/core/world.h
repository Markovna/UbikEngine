#pragma once

#include "base/math.h"
#include "base/slot_map.h"
#include "core/ecs.h"
#include "core/assets/assets.h"

#include <vector>
#include <unordered_map>

using entity_id = ecs::entity;

struct entity {
  entity_id id = ecs::invalid;

  [[nodiscard]] bool is_valid() const { return id != ecs::invalid; }

  static const entity& invalid() {
    static const entity invalid{};
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

template<>
struct serializer<transform_component> {
  static void from_asset(assets::repository*, const asset&, transform_component&);
  static void to_asset(asset&, const transform_component&);
};

namespace ecs {

  class system {
   public:
    virtual ~system() = default;
  };

}

class world : ecs::registry {
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

  entity load_from_asset(assets::repository*, const asset& asset, entity parent = entity::invalid(), entity next = entity::invalid());

  void save_to_asset(asset& asset);
  void save_to_asset(asset& asset, entity entity);

  void set_parent(entity ent, entity parent, entity next = entity::invalid());

  [[nodiscard]] entity parent(entity ent) const;

  [[nodiscard]] transform local_transform(entity entity) const;
  [[nodiscard]] vec3 local_position(entity entity) const;
  [[nodiscard]] quat local_rotation(entity entity) const;

  void set_local_transform(entity entity, const transform& local);
  void set_local_position(entity entity, const vec3& pos);
  void set_local_rotation(entity entity, const quat& rot);

  [[nodiscard]] transform world_transform(entity entity) const;
  void set_world_transform(entity ent, const transform& world);

  [[nodiscard]] entity root() const { return ecs::registry::get<link_component>(root_.id).child; }
  [[nodiscard]] size_t roots_size() const { return ecs::registry::get<link_component>(root_.id).children_size; }

  [[nodiscard]] entity child(entity entity) const {
    return ecs::registry::get<link_component>(entity.id).child;
  }

  [[nodiscard]] bool is_child_of(entity ent, entity parent_candidate) const {
    if (parent_candidate == entity::invalid()) return true;

    while (ent) {
      const auto& comp = ecs::registry::get<link_component>(ent.id);
      if (comp.parent == parent_candidate)
        return true;

      ent = comp.parent;
    }
    return false;
  }

  [[nodiscard]] entity prev(entity entity) const {
    return ecs::registry::get<link_component>(entity.id).prev;
  }

  [[nodiscard]] entity next(entity entity) const {
    return ecs::registry::get<link_component>(entity.id).next;
  }

  [[nodiscard]] size_t children_size(entity entity) const {
    return ecs::registry::get<link_component>(entity.id).children_size;
  }

  template<class Component, class ...Args>
  Component& add_component(entity entity, Args&&... args) {
    return ecs::registry::emplace<Component>(entity.id, std::forward<Args>(args)...);
  }

  template<class Component>
  void remove_component(entity entity) {
    static_assert(!std::is_same_v<Component, transform_component>, "Trying to remove transform_component");
    static_assert(!std::is_same_v<Component, link_component>, "Trying to remove link_component");

    ecs::registry::remove<Component>(entity.id);
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

  template<class Component>
  Component& component(entity entity) {
    return ecs::registry::get<Component>(entity.id);
  }

  template<class Component>
  [[nodiscard]] const Component& component(entity entity) const {
    return ecs::registry::get<Component>(entity.id);
  }

  template<class Component>
  const Component* try_get_component(entity entity) const {
    return ecs::registry::try_get<Component>(entity.id);
  }

  template<class Component>
  Component* try_get_component(entity entity) {
    return ecs::registry::try_get<Component>(entity.id);
  }

  template<class Component>
  [[nodiscard]] bool has_component(entity entity) const {
    return ecs::registry::has<Component>(entity.id);
  }

  template<class ...Component>
  ecs::component_view<Component...> view() {
    return ecs::registry::view<Component...>();
  }

  [[nodiscard]] bool valid(entity e) const {
    return ecs::registry::valid(e.id);
  }

 private:
  void set_parent_impl(entity ent, entity parent, entity next);

  [[nodiscard]] const transform& resolve_transform(entity ent) const {
    const transform_component& component = ecs::registry::get<transform_component>(ent.id);
    if (component.dirty) {
      entity p = parent(ent);

      component.world = (p ? world_transform(p) : transform::identity()) * component.local;
      component.dirty = false;
    }
    return component.world;
  }

  void set_transform_dirty(entity ent, bool dirty) const {
    ecs::registry::get<transform_component>(ent.id).dirty = dirty;

    if (dirty) {
      entity c = child(ent);
      while (c) {
        set_transform_dirty(c, dirty);
        c = next(c);
      }
    }
  }

 private:
  entity root_;
  std::vector<system_info> systems_;
  std::unordered_map<meta::typeid_t, uint32_t> systems_index_;
};

#define register_component_i(__type) meta::registration::interface<__type>(component_i { \
    .instantiate = component_instantiate_impl<__type> \
});

struct component_i {
  void* (*instantiate)(world& w, const entity& e);
};

template<class, class = void>
struct is_instantiable_component : std::false_type {};

template<class T>
struct is_instantiable_component<T, std::void_t<decltype(std::declval<world>().add_component<T>(std::declval<entity>()))>> : std::true_type {};

template<class T>
inline constexpr bool is_instantiable_component_v = is_instantiable_component<T>::value;

template<class Component>
static void* component_instantiate_impl(world& w, const entity& e) {
  static_assert(is_instantiable_component_v<Component>, "Couldn't instantiate type.");
  return static_cast<void*>(&w.add_component<Component>(e));
}

namespace ecs {

extern world *world;

void init_world();
void shutdown_world();

}


