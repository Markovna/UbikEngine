#include "sandbox_plugin.h"
#include "spin_plugin.h"
#include "core/world.h"
#include "base/log.h"
#include "core/components/mesh_component.h"
#include "core/components/camera_component.h"
#include "core/assets/assets.h"
#include "core/plugins.h"

#include "core/meta/registration.h"

class test : public world_system {
 public:
  int count = 0;
  void stop(world*) override {}
  void start(world* w) override {
    logger::core::Info("test::start");

    assets::handle scene =
        assets::load(
            assets::g_provider,
            fs::absolute("assets/scenes/start_scene.entity")
          );
    w->load_from_asset(*scene);

    /*
    auto root_ent = e->world->create_entity();
    auto& ent0_tr = e->world->component<transform_component>(root_ent);
    auto& ent0_cc = e->world->add_component<custom_component>(root_ent);
    ent0_cc.a = 42;

    auto mesh_ent = e->world->create_entity({}, root_ent);
    auto cam_ent = e->world->create_entity({});

    e->world->add_component<mesh_component>(mesh_ent);

//    for (int i = 1; i < 2; i++) {
//      {
//        vec3 pos = (i * 2.0f + 1) * vec3::forward() + (i * 0.7f) * vec3::right();
//        auto ent = e->world->create_entity({}, root_ent);
//        e->world->add_component<mesh_component>(ent);
//        e->world->set_local_transform(ent, transform::from_matrix(mat4::translation(pos)));
//      }
//      {
//        vec3 pos = (i * 2.0f + 1) * vec3::forward() + (i * 0.7f) * -vec3::right();
//        auto ent = e->world->create_entity({}, root_ent);
//        e->world->add_component<mesh_component>(ent);
//        e->world->set_local_transform(ent, transform::from_matrix(mat4::translation(pos)));
//        e->world->add_component<custom_component>(ent).a = 42;
//      }
//    }

    auto& cam = e->world->add_component<camera_component>(cam_ent);
    e->world->set_local_transform(cam_ent, transform::from_matrix(mat4::translation({0,-2.0f,-3.0f})));

    */
  }

  void update(world* w) override {
    auto mesh_view = w->view<mesh_component, custom_component>();
    for (ecs::entity id : mesh_view) {
      mesh_view.get<mesh_component>(id).set_color(color::green());
    }

//    logger::core::Info(meta::get_type<test_component>().name());
  }
};

void load_sandbox_plugin(plugins* plugins_registry) {

  register_type(test_component);
  register_type(inner_type);
  register_type(custom_component);

  logger::core::Info("load_sandbox_plugin");
  ecs::world->register_system<test>("test");
}

void unload_sandbox_plugin(plugins* plugins_registry) {
  logger::core::Info("unload_sandbox_plugin");
  ecs::world->unregister_system("test");
}

void serializer<custom_component>::from_asset(const asset &asset, custom_component& comp) {
  assets::get(asset, "a", comp.a);
  assets::get(asset, "b", comp.b);
  assets::get(asset, "c", comp.c);
}

void serializer<custom_component>::to_asset(asset &asset, const custom_component& comp) {
  assets::set(asset, "a", comp.a);
  assets::set(asset, "b", comp.b);
  assets::set(asset, "c", comp.c);
}

void serializer<inner_type>::from_asset(const asset &asset, inner_type& val) {
  assets::get(asset, "a", val.a);
}

void serializer<inner_type>::to_asset(asset &asset, const inner_type& val) {
  assets::set(asset, "a", val.a);
}