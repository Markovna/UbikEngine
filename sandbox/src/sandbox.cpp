#include "core/components/mesh_component.h"
#include "base/color.h"
#include "sandbox.h"
#include "core/application.h"
#include "base/log.h"
#include "core/assets/assets.h"
#include "core/world.h"
#include "sandbox_plugin.h"

class application_impl : public application {
 public:
  void start(assets::repository* rep) override {
    logger::core::Info("application_impl::start");

    assets::handle scene = rep->load(fs::absolute("assets/scenes/start_scene.entity"));
    ecs::world->load_from_asset(rep, *scene);
  }

  void tick() override {
    auto mesh_view = ecs::world->view<mesh_component, custom_component>();
    for (ecs::entity id : mesh_view) {
      mesh_view.get<mesh_component>(id).set_color(color::red());
    }
  }

  void stop() override {}
};

void load_sandbox(struct plugins*) {
  set_application(new application_impl);
}

void unload_sandbox(struct plugins*) {
  remove_application();
}