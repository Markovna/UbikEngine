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

class world : public ecs::registry {
 public:
  static std::unique_ptr<world> create() { return std::make_unique<world>(); }

  world();

  entity create_entity(const transform& local = {}, entity parent = entity::invalid(), entity next = entity::invalid());
  void destroy_entity(entity entity);

  entity load_from_asset(const class asset& asset, entity parent = entity::invalid(), entity next = entity::invalid());

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

 private:
  void set_parent_impl(entity ent, entity parent, entity next);
  [[nodiscard]] const transform& resolve_transform(entity ent) const;
  void set_transform_dirty(entity ent, bool dirty) const;

 private:
  entity root_;
};

template<class T>
void instantiate_component(world& world, entity& e) {
  world.emplace<T>(e.id);
}

void init_world(const struct systems_registry&);

void propagate_asset_changes(world& world, class asset_repository& repository);

void resolve_transforms(const world&, entity);