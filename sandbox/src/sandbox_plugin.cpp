#include "sandbox_plugin.h"
#include "spin_plugin.h"
#include "core/world.h"
#include "base/log.h"
#include "core/components/mesh_component.h"
#include "core/components/camera_component.h"
#include "core/assets/assets.h"
#include "core/plugins.h"

#include "core/meta/registration.h"

class test : public ecs::system {
 public:
  int count = 0;

  void update(world* w, ecs::component_view<mesh_component, const custom_component> view) {
    for (ecs::entity id : view) {
      view.get<mesh_component>(id).set_color(color::red());

    }
  }
};

void load_sandbox_plugin(plugins* plugins_registry) {

  register_type(test_component);
  register_type(inner_type);
  register_type(custom_component);
  register_type(test);

  register_component_i(test_component);
  register_component_i(custom_component);

  register_serializer_i(test_component);
  register_serializer_i(custom_component);

  logger::core::Info("load_sandbox_plugin");

  ecs::world->register_system<test>();
}

void unload_sandbox_plugin(plugins* plugins_registry) {
  logger::core::Info("unload_sandbox_plugin");

  ecs::world->remove_system<test>();
}

void serializer<custom_component>::from_asset(assets::provider* p, const asset &asset, custom_component& comp) {
  assets::get(p, asset, "a", comp.a);
  assets::get(p, asset, "b", comp.b);
  assets::get(p, asset, "c", comp.c);
}

void serializer<custom_component>::to_asset(asset &asset, const custom_component& comp) {
  assets::set(asset, "a", comp.a);
  assets::set(asset, "b", comp.b);
  assets::set(asset, "c", comp.c);
}

void serializer<inner_type>::from_asset(assets::provider* p, const asset &asset, inner_type& val) {
  assets::get(p, asset, "a", val.a);
}

void serializer<inner_type>::to_asset(asset &asset, const inner_type& val) {
  assets::set(asset, "a", val.a);
}