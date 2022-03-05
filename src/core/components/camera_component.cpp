#include "camera_component.h"
#include "core/meta/registration.h"
#include "core/component_loader.h"
#include "core/meta/interface_registry.h"
#include "core/systems_registry.h"
#include "core/asset_repository.h"
#include "core/world.h"

void load_camera_component(const asset& asset, world& world, entity& e) {
  auto& comp = world.get<camera_component>(e.id);
  comp.fov = asset.at("fov");
  comp.near = asset.at("near");
  comp.far = asset.at("far");
  comp.orthogonal_size = asset.at("orthogonal_size");

  const ::asset& rect = asset.at("normalized_rect");

  comp.normalized_rect.x = rect.at("x");
  comp.normalized_rect.y = rect.at("y");
  comp.normalized_rect.z = rect.at("z");
  comp.normalized_rect.w = rect.at("w");

  comp.clear_color = color::black();
}

void register_camera_component(struct systems_registry& registry) {
  auto type = meta::registration::type<camera_component>();
  auto interface_registry = registry.get<::interface_registry>();
  interface_registry->register_interface(type.id(), load_component_interface(load_camera_component));
  interface_registry->register_interface(type.id(), instantiate_component_interface(instantiate_component<camera_component>));
}