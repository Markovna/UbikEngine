#include <core/meta/interface_registry.h>
#include "transform_component.h"
#include "core/systems_registry.h"
#include "core/meta/registration.h"
#include "core/asset_repository.h"
#include "core/component_loader.h"
#include "core/world.h"

void load_transform_component(const asset& asset, world& world, entity& e) {
  auto& comp = world.get<transform_component>(e.id);

  const ::asset& position = asset.at("position");
  comp.local.position.x = position.at("x").get<float>();
  comp.local.position.y = position.at("y").get<float>();
  comp.local.position.z = position.at("z").get<float>();

  const ::asset& rotation = asset.at("rotation");
  comp.local.rotation.x = rotation.at("x").get<float>();
  comp.local.rotation.y = rotation.at("y").get<float>();
  comp.local.rotation.z = rotation.at("z").get<float>();
  comp.local.rotation.w = rotation.at("w").get<float>();

  const ::asset& scale = asset.at("scale");
  comp.local.scale.x = scale.at("x").get<float>();
  comp.local.scale.y = scale.at("y").get<float>();
  comp.local.scale.z = scale.at("z").get<float>();

  comp.dirty = true;
}

void register_transform_component(struct systems_registry& registry) {
  auto type = meta::registration::type<transform_component>();
  auto interface_registry = registry.get<::interface_registry>();
  interface_registry->register_interface(type.id(), load_component_interface(load_transform_component));
  interface_registry->register_interface(type.id(), instantiate_component_interface(instantiate_component<transform_component>));
}