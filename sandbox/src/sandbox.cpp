#include <editor/editor_tab.h>
#include "sandbox.h"
#include "core/application.h"
#include "core/simulation_events.h"
#include "core/simulation.h"
#include "core/assets_filesystem.h"
#include "core/components/mesh_component.h"
#include "core/components/transform_component.h"
#include "core/world.h"
#include "core/systems_registry.h"

static system_ptr<::asset_repository> assets_repository;

struct application_impl : public application {
  void start() override {
  }
  void tick() override {
  }
  void stop() override {
  }
};

class sandbox_simulation : public simulation {
 public:
  void start(world& world) override {
    world_ = &world;
    world.load_from_asset(*assets_repository->get_asset("assets/scenes/start_scene.entity"));
  }

  void update(world& world, float dt) override {
    assert(world_);

    time += dt;
    auto mesh_view = world_->view<transform_component, mesh_component>();
    for (auto entity : mesh_view) {

      static vec3 axis = vec3::normalized({1,1,1});
      quat rot = quat::axis(axis, time * 0.001f);

      world_->set_local_rotation({ entity }, rot);
      world_->world_transform({ entity });
    }
  }

  void stop() override {}

  ~sandbox_simulation() override {
    std::cout << "sandbox_simulation destroyed 34\n";
  }

  void load(world* w) {
    world_ = w;
  }

  const world* get_world() const { return world_; }

 private:
  float time = 0;
  world* world_;
};

static sandbox_simulation* sandbox_sim;

void load_plugin(std::istringstream* in, systems_registry& reg) {

  assets_repository = reg.get<struct asset_repository>();

  reg.set<application>(std::make_unique<application_impl>());
//  reg.add<editor_tab>(std::make_unique<sandbox_editor_tab>());

  sandbox_sim = reg.add<simulation>(std::make_unique<sandbox_simulation>());

  if (in) {

    uint64_t address;
    in->read((char*) &address, sizeof(uint64_t));

    sandbox_sim->load((world*) (intptr_t) address);
  }
}

void unload_plugin(std::ostringstream* out, systems_registry& reg) {
  if (out) {
    uint64_t address = (intptr_t) sandbox_sim->get_world();
    out->write((char*) &address, sizeof(uint64_t));
  }

  reg.remove<application>();
  reg.erase<simulation>(sandbox_sim);
}