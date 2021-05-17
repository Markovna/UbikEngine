#include "sandbox_plugin.h"
#include "spin_plugin.h"
#include "core/engine.h"
#include "core/world.h"
#include "core/components/mesh_component.h"
#include "core/components/camera_component.h"

class test : public plugin_base {
 public:
  int count = 0;
  void stop(engine *e) override {}
  void start(engine *e) override {
    logger::core::Info("test::start");

    auto root_ent = e->world->create_entity();
    auto& ent0_tr = e->world->component<transform_component>(root_ent);

    auto mesh_ent = e->world->create_entity({}, root_ent);
    auto cam_ent = e->world->create_entity({});

    e->world->add_component<mesh_component>(mesh_ent);

    for (int i = 1; i < 10; i++) {
      {
        vec3 pos = (i * 2.0f + 1) * vec3::forward() + (i * 0.7f) * vec3::right();
        auto ent = e->world->create_entity({}, root_ent);
        e->world->add_component<mesh_component>(ent);
        e->world->set_local_transform(ent, transform::from_matrix(mat4::translation(pos)));
      }
      {
        vec3 pos = (i * 2.0f + 1) * vec3::forward() + (i * 0.7f) * -vec3::right();
        auto ent = e->world->create_entity({}, root_ent);
        e->world->add_component<mesh_component>(ent);
        e->world->set_local_transform(ent, transform::from_matrix(mat4::translation(pos)));
      }
    }

    auto& cam = e->world->add_component<camera_component>(cam_ent);
    e->world->set_local_transform(cam_ent, transform::from_matrix(mat4::translation({0,-2.0f,-3.0f})));
  }

  void update(engine *e) override {
    count++;

    if (count > 180) {
      some_plugin *pl = e->plugins->get_plugin<some_plugin>("some_plugin");
      if (pl) {
        int c = pl->foo();
        pl->start(e);
        logger::core::Info("test::update {}", c);

      }
      count = 0;
    }
  }
};


void load_sandbox_plugin(plugins_registry *reg) {
  logger::core::Info("load_sandbox_plugin");
  reg->add_plugin<test>("test");
}


void unload_sandbox_plugin(plugins_registry *reg) {
  logger::core::Info("unload_sandbox_plugin");
  reg->remove_plugin("test");
}