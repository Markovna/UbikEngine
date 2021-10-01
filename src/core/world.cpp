#include "world.h"
#include "meta/type.h"
#include "core/serialization.h"

void world::set_parent_impl(entity ent, entity parent, entity next) {
  link_component& comp = component<link_component>(ent);
  if (comp.parent) {
    link_component& parent_comp = component<link_component>(comp.parent);
    parent_comp.children_size--;

    if (parent_comp.child == ent)
      parent_comp.child = comp.next;

    if (parent_comp.child_last == ent)
      parent_comp.child_last = comp.prev;
  }

  if (comp.prev)
    component<link_component>(comp.prev).next = comp.next;

  if (comp.next)
    component<link_component>(comp.next).prev = comp.prev;

  comp.parent = parent;
  comp.next = next;
  comp.prev = entity::invalid();

  if (comp.parent) {
    link_component& parent_comp = component<link_component>(comp.parent);
    parent_comp.children_size++;

    if (!comp.next) {
      if (parent_comp.child_last) {
        link_component& last_child_comp = component<link_component>(parent_comp.child_last);
        last_child_comp.next = ent;
      }

      comp.prev = parent_comp.child_last;
      parent_comp.child_last = ent;
    }

    if (comp.next == parent_comp.child)
      parent_comp.child = ent;
  }

  if (comp.next) {
    link_component& next_comp = component<link_component>(comp.next);

    if (next_comp.prev) {
      link_component &prev_comp = component<link_component>(next_comp.prev);
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

entity world::load_from_asset(assets::provider* provider, const asset& asset, entity parent, entity next) {
  entity entity { ecs::registry::create() };
  ecs::registry::emplace<link_component>(entity.id);

  for (const ::asset& comp_asset : asset.at("components")) {
    std::string name = comp_asset.at("__type");
    meta::type type =  meta::get_type(name.c_str());
    meta::get_interface<serializer_i>(type.id())->from_asset(
        provider,
        comp_asset,
        meta::get_interface<component_i>(type.id())->instantiate(*this, entity)
      );
  }

  set_parent(entity, parent, next);

  if (asset.contains("children")) {
    for (const ::asset& child_asset : asset.at("children")) {
      load_from_asset(provider, child_asset, entity);
    }
  }
  return entity;
}

void world::save_to_asset(asset& asset, entity e) {
  asset["__type"] = "entity";
  asset["__guid"] = guid::generate();

  for (auto [id, ptr] : ecs::registry::get_components(e.id)) {
    meta::get_interface<serializer_i>(id)->to_asset(asset["components"], ptr);
  }

  entity child_e = child(e);
  while (child_e) {
    ::asset& child_asset = asset["children"].emplace_back();
    save_to_asset(child_asset, child_e);
    child_e = next(child_e);
  }
}

void world::save_to_asset(asset &asset) {
  save_to_asset(asset, root_);
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
  transform_component& component = ecs::registry::get<transform_component>(entity.id);
  component.local = local;
  set_transform_dirty(entity, true);
}

void world::set_local_position(entity entity, const vec3 &pos) {
  transform_component& component = ecs::registry::get<transform_component>(entity.id);
  component.local.position = pos;
  set_transform_dirty(entity, true);
}

void world::set_local_rotation(entity entity, const quat &rot) {
  transform_component& component = ecs::registry::get<transform_component>(entity.id);
  component.local.rotation = rot;
  set_transform_dirty(entity, true);
}

transform world::world_transform(entity entity) const {
  return resolve_transform(entity);
}

void world::set_world_transform(entity ent, const transform &world) {
  entity p = parent(ent);
  transform parent_inv = p ? transform::inverse(world_transform(p)) : transform::identity();

  transform_component& component = ecs::registry::get<transform_component>(ent.id);
  component.local = parent_inv * world;
  component.world = world;
  component.dirty = false;
}

void serializer<transform_component>::from_asset(assets::provider* p, const asset& asset, transform_component& comp) {
  assets::get(p, asset, "position", comp.local.position);
  assets::get(p, asset, "rotation", comp.local.rotation);
  assets::get(p, asset, "scale", comp.local.scale);

  comp.dirty = true;
}

void serializer<transform_component>::to_asset(asset& asset, const transform_component& comp) {
  assets::set(asset, "position", comp.local.position);
  assets::set(asset, "rotation", comp.local.rotation);
  assets::set(asset, "scale", comp.local.scale);
}

namespace ecs {

class world* world;

void init_world() {
  world = new ::world;
}

void shutdown_world() {
  delete world;
}

}