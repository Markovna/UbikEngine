#include "world.h"
#include "meta/type.h"
#include "core/component_loader.h"
#include "base/log.h"
#include "core/components/transform_component.h"
#include "systems_registry.h"
#include "core/components/version_component.h"
#include "core/meta/interface_registry.h"

static system_ptr<::interface_registry> interface_reg;

void world::set_parent_impl(entity ent, entity parent, entity next) {
  auto& comp = get<link_component>(ent.id);
  if (comp.parent) {
    auto& parent_comp = get<link_component>(comp.parent.id);
    parent_comp.children_size--;

    if (parent_comp.child == ent)
      parent_comp.child = comp.next;

    if (parent_comp.child_last == ent)
      parent_comp.child_last = comp.prev;
  }

  if (comp.prev)
    get<link_component>(comp.prev.id).next = comp.next;

  if (comp.next)
    get<link_component>(comp.next.id).prev = comp.prev;

  comp.parent = parent;
  comp.next = next;
  comp.prev = entity::invalid();

  if (comp.parent) {
    auto& parent_comp = get<link_component>(comp.parent.id);
    parent_comp.children_size++;

    if (!comp.next) {
      if (parent_comp.child_last) {
        auto& last_child_comp = get<link_component>(parent_comp.child_last.id);
        last_child_comp.next = ent;
      }

      comp.prev = parent_comp.child_last;
      parent_comp.child_last = ent;
    }

    if (comp.next == parent_comp.child)
      parent_comp.child = ent;
  }

  if (comp.next) {
    auto& next_comp = get<link_component>(comp.next.id);

    if (next_comp.prev) {
      auto &prev_comp = get<link_component>(next_comp.prev.id);
      prev_comp.next = ent;
      comp.prev = next_comp.prev;
    }

    next_comp.prev = ent;
  }
}

entity world::create_entity(const transform &local, entity parent, entity next) {
  entity entity { ecs::registry::create() };
  ecs::registry::emplace<transform_component>(entity.id).local = local;
  ecs::registry::emplace<link_component>(entity.id);

  set_parent(entity, parent, next);
  return entity;
}

void world::destroy_entity(entity entity) {

  set_parent_impl(entity, entity::invalid(), entity::invalid());

  while (children_size(entity)) {
    destroy_entity(child(entity));
  }

  ecs::registry::destroy(entity.id);
}

void world::set_parent(entity ent, entity parent, entity next) {
  transform world = world_transform(ent);
  set_parent_impl(ent, parent ? parent : root_, next);
  set_world_transform(ent, world);
}

void propagate_asset_changes(world& world, asset_repository& repository) {
  auto version_view = world.view<version_component>();
  if (!version_view.size())
    return;

  std::vector<entity> stack;
  auto& root_version = version_view.get(world.root().id);
  asset* root_asset = repository.get_asset(root_version.id);
  if (root_asset && root_version.version != root_asset->version()) {
    stack.push_back(world.root());
  }

  std::vector<entity> modified;
  while (!stack.empty()) {
    entity e = stack.back();
    stack.pop_back();

    auto& version = version_view.get(e.id);
    asset* entity_asset = repository.get_asset(version.id);

    const class asset& components = entity_asset->at("components");
    if (components.version() != version.components_version) {
      modified.push_back(e);
    }

    // TODO: check added/removed children
    // const asset_array& children = asset->at("children");

    for (auto child = world.child(e); child; child = world.next(child)) {
      auto& child_version = version_view.get(child.id);
      asset* child_asset = repository.get_asset(child_version.id);
      if (child_asset && child_version.version != child_asset->version()) {
        stack.push_back(child);
      }
    }

    version.version = entity_asset->version();
  }

  for (auto e : modified) {
    auto& version = version_view.get(e.id);
    asset* entity_asset = repository.get_asset(version.id);

    const asset& components = entity_asset->at("components");
    for (auto& [type_name, comp_asset] : components) {
      meta::type type = meta::get_type(type_name.c_str());
      if (!type.is_valid()) {
        logger::core::Warning("Unknown component type {}", type_name);
        continue;
      }

      auto* load = interface_reg->get_interface<load_component_interface>(type.id());
      if (load) {
        load->invoke(comp_asset, world, e);
      }
    }

    version.components_version = components.version();
  }
}

entity world::load_from_asset(const asset& asset, entity parent, entity next) {
  entity entity { ecs::registry::create() };
  ecs::registry::emplace<link_component>(entity.id);

  const class asset& components = asset.at("components");
  auto& version = ecs::registry::emplace<version_component>(entity.id);
  version.id = asset.id();
  version.version = asset.version();
  version.components_version = components.version();

  for (auto& [type_name, comp_asset] : components) {
    meta::type type = meta::get_type(type_name.c_str());
    if (!type.is_valid()) {
      logger::core::Warning("Unknown component type {}", type_name);
      continue;
    }

    auto* instantiate = interface_reg->get_interface<instantiate_component_interface>(type.id());
    if (instantiate) {
      instantiate->invoke(*this, entity);
      auto* load = interface_reg->get_interface<load_component_interface>(type.id());
      if (load) {
        load->invoke(comp_asset, *this, entity);
      } else {
        logger::core::Warning("Couldn't find component loader for type {}", type_name);
      }
    } else {
      logger::core::Warning("Couldn't find component loader for type {}", type_name);
    }
  }

  set_parent(entity, parent, next);

  if (asset.contains("children")) {
    for (const ::asset& child_asset : asset.at("children").get<asset_array&>()) {
      load_from_asset(child_asset, entity);
    }
  }
  return entity;
}

world::world() : root_({ecs::registry::create()}) {
  ecs::registry::emplace<transform_component>(root_.id);
  ecs::registry::emplace<link_component>(root_.id);
}

entity world::parent(entity ent) const {
  entity parent = ecs::registry::get<link_component>(ent.id).parent;
  return parent == root_ ? entity::invalid() : parent;
}

transform world::local_transform(entity entity) const {
  return ecs::registry::get<transform_component>(entity.id).local;
}

vec3 world::local_position(entity entity) const {
  return ecs::registry::get<transform_component>(entity.id).local.position;
}

quat world::local_rotation(entity entity) const {
  return ecs::registry::get<transform_component>(entity.id).local.rotation;
}

void world::set_local_transform(entity entity, const transform &local) {
  auto& component = ecs::registry::get<transform_component>(entity.id);
  component.local = local;
  set_transform_dirty(entity, true);
}

void world::set_local_position(entity entity, const vec3 &pos) {
  auto& component = ecs::registry::get<transform_component>(entity.id);
  component.local.position = pos;
  set_transform_dirty(entity, true);
}

void world::set_local_rotation(entity entity, const quat &rot) {
  auto& component = ecs::registry::get<transform_component>(entity.id);
  component.local.rotation = rot;
  set_transform_dirty(entity, true);
}

transform world::world_transform(entity entity) const {
  return resolve_transform(entity);
}

void world::set_world_transform(entity ent, const transform &world) {
  entity p = parent(ent);
  transform parent_inv = p ? transform::inverse(world_transform(p)) : transform::identity();

  auto& component = ecs::registry::get<transform_component>(ent.id);
  component.local = parent_inv * world;
  component.world = world;
  component.dirty = false;
}

const transform &world::resolve_transform(entity ent) const {
  const auto& component = ecs::registry::get<transform_component>(ent.id);
  if (component.dirty) {
    entity p = parent(ent);

    component.world = (p ? world_transform(p) : transform::identity()) * component.local;
    component.dirty = false;
  }
  return component.world;
}

void world::set_transform_dirty(entity ent, bool dirty) const {
  ecs::registry::get<transform_component>(ent.id).dirty = dirty;

  if (dirty) {
    entity c = child(ent);
    while (c) {
      set_transform_dirty(c, dirty);
      c = next(c);
    }
  }
}

bool world::is_child_of(entity ent, entity parent_candidate) const {
  if (parent_candidate == entity::invalid()) return true;

  while (ent) {
    const auto& comp = ecs::registry::get<link_component>(ent.id);
    if (comp.parent == parent_candidate)
      return true;

    ent = comp.parent;
  }
  return false;
}

void init_world(const systems_registry& registry) {
  interface_reg = registry.get<::interface_registry>();
}

void resolve_transforms(const world& w, entity root) {
  auto transform_view = w.view<transform_component>();

  std::vector<entity> stack;
  stack.push_back(root);

  while (!stack.empty()) {
    entity e = stack.back();
    stack.pop_back();

    auto& transform = transform_view.get(e.id);
    bool dirty = transform.dirty;
    if (dirty) {
      entity parent = w.parent(e);
      transform.world = (parent ? transform_view.get(parent.id).world : transform::identity()) * transform.local;
      transform.dirty = false;
    }

    for (auto child = w.child(e); child; child = w.next(child)) {
      auto& child_transform = transform_view.get(child.id);
      child_transform.dirty |= dirty;

      stack.push_back(child);
    }
  }
}