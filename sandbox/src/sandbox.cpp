#include "sandbox.h"
#include "core/application.h"
#include "core/simulation_events.h"
#include "core/simulation.h"
#include "gfx/experimental/asset_repository.h"
#include "gfx/experimental/mesh_component.h"
#include "core/world.h"
#include "gfx/experimental/systems_registry.h"

static system_ptr<::assets_repository> assets_repository;

struct application_impl : public application {
  void start() override {
    std::cout << "start\n";
  }
  void tick() override {
    std::cout << "tick\n";
  }
  void stop() override {
    std::cout << "stop\n";
  }
};

class sandbox_simulation : public simulation {
 public:
  void start(world& world) override {
    world.load_from_asset(*assets_repository->load("assets/scenes/start_scene.entity"));
  }

  void update(world& world, float dt) override {
    std::cout << "sandbox_simulation::update 5\n";

    time += dt;
    auto mesh_view = world.view<transform_component, mesh_component>();
    for (auto entity : mesh_view) {

      static vec3 axis = vec3::normalized({1,1,1});
      quat rot = quat::axis(axis, time * 0.001f);

      world.set_local_rotation({ entity }, rot);
      world.world_transform({ entity });
    }
  }

  void stop() override {}

  ~sandbox_simulation() override {
    std::cout << "sandbox_simulation destroyed 5\n";
  }

 private:
  float time = 0;
};

static sandbox_simulation* sandbox_sim;

void load_sandbox(systems_registry& reg) {
  std::cout << "load_sandbox\n";

  reg.set<application>(std::make_unique<application_impl>());

  assets_repository = reg.get<struct assets_repository>();

  sandbox_sim = reg.add<simulation>(std::make_unique<sandbox_simulation>());
}

void unload_sandbox(systems_registry& reg) {
  std::cout << "unload_sandbox\n";

  reg.erase<simulation>(sandbox_sim);
}