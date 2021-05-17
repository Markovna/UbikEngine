#pragma once

#include "base/math.h"
#include "base/int_set.h"
#include "core/ecs.h"
#include "gfx/gfx.h"

struct entity {
  uint32_t id = ecs::invalid;

  [[nodiscard]] bool is_valid() const { return id != ecs::invalid; }

  static entity invalid() {
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
  uint32_t children_size;
};

struct transform_component {
  transform local;
  mutable transform world;
  mutable bool dirty = false;
};

class world : ecs::registry {
 public:
  static std::unique_ptr<world> Create() { return std::make_unique<world>(); }

  world() : root_({ecs::registry::create()}) {
    ecs::registry::emplace<transform_component>(root_.id);
    ecs::registry::emplace<link_component>(root_.id);
  }

  entity create_entity(const transform& local = {}, entity parent = entity::invalid(), entity next = entity::invalid()) {
    entity entity { ecs::registry::create() };
    ecs::registry::emplace<transform_component>(entity.id).local = local;
    ecs::registry::emplace<link_component>(entity.id);

    set_parent(entity, parent, next);
    return entity;
  }

  void destroy_entity(entity entity) {

    set_parent_impl(entity, entity::invalid(), entity::invalid());

    while (children_size(entity)) {
      destroy_entity(child(entity));
    }

    ecs::registry::destroy(entity.id);
  }

  void set_parent(entity ent, entity parent, entity next = entity::invalid()) {
    transform world = world_transform(ent);
    set_parent_impl(ent, parent ? parent : root_, next);
    set_world_transform(ent, world);
  }

  [[nodiscard]] entity parent(entity ent) const {
    entity parent = ecs::registry::get<link_component>(ent.id).parent;
    return parent == root_ ? entity::invalid() : parent;
  }

  [[nodiscard]] transform local_transform(entity entity) const {
    return ecs::registry::get<transform_component>(entity.id).local;
  }

  [[nodiscard]] vec3 local_position(entity entity) const {
    return ecs::registry::get<transform_component>(entity.id).local.position;
  }

  [[nodiscard]] quat local_rotation(entity entity) const {
    return ecs::registry::get<transform_component>(entity.id).local.rotation;
  }

  void set_local_transform(entity entity, const transform& local) {
    transform_component& component = ecs::registry::get<transform_component>(entity.id);
    component.local = local;
    set_transform_dirty(entity, true);
  }

  void set_local_position(entity entity, const vec3& pos) {
    transform_component& component = ecs::registry::get<transform_component>(entity.id);
    component.local.position = pos;
    set_transform_dirty(entity, true);
  }

  void set_local_rotation(entity entity, const quat& rot) {
    transform_component& component = ecs::registry::get<transform_component>(entity.id);
    component.local.rotation = rot;
    set_transform_dirty(entity, true);
  }

  [[nodiscard]] transform world_transform(entity entity) const {
    return resolve_transform(entity);
  }

  void set_world_transform(entity ent, const transform& world) {
    entity p = parent(ent);
    transform parent_inv = p ? transform::inverse(world_transform(p)) : transform::identity();

    transform_component& component = ecs::registry::get<transform_component>(ent.id);
    component.local = parent_inv * world;
    component.world = world;
    component.dirty = false;
  }

  [[nodiscard]] entity root() const { return ecs::registry::get<link_component>(root_.id).child; }
  [[nodiscard]] size_t roots_size() const { return ecs::registry::get<link_component>(root_.id).children_size; }

  [[nodiscard]] entity child(entity entity) const {
    return ecs::registry::get<link_component>(entity.id).child;
  }

  [[nodiscard]] bool is_child_of(entity ent, entity parent_candidate) const {
    if (parent_candidate == entity::invalid()) return true;

    while (ent) {
      const link_component& comp = ecs::registry::get<link_component>(ent.id);
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
  Component& add_component(entity entity, Args &&... args) {
    return ecs::registry::emplace<Component>(entity.id, std::forward<Args>(args)...);
  }

  template<class Component>
  void remove_component(entity entity) {
    static_assert(!std::is_same_v<Component, transform_component>, "Trying to remove TransformComponent");
    static_assert(!std::is_same_v<Component, link_component>, "Trying to remove LinkComponent");

    ecs::registry::remove<Component>(entity.id);
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

//  template<typename Func>
//  void visit(entity entity, Func func) const {
//    ecs::registry::visit(entity.id, func);
//  }

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
};


