#include "spin_plugin.h"

#include "base/math.h"
#include "base/log.h"

#include "core/plugins.h"
#include "core/input_system.h"
#include "core/world.h"
#include "core/meta/registration.h"

class spin_plugin : public ecs::system {
 public:
  static int ver;
  int count = 0;
  int counter = 0;

  spin_plugin() = default;

  void update(world*, ecs::component_view<const transform_component>) {
    count++;

    if (count > 180) {
      logger::core::Info("spin_plugin::update (ver.{}) {}", ver, counter++);
      count = 0;
    }
  }

  ~spin_plugin() override {
    logger::core::Info("~spin_plugin (ver.{})", ver);
  }
};

int spin_plugin::ver = 15;

void load_spin_plugin(plugins* plugins_registry) {

  logger::core::Info("load_spin_plugin {}", spin_plugin::ver);

  register_type(spin_plugin);
  register_type(some_plugin);

  ecs::world->register_system<spin_plugin>();
  ecs::world->register_system<some_plugin>();
}

void unload_spin_plugin(plugins* plugins_registry) {
  logger::core::Info("unload_spin_plugin {}", spin_plugin::ver);

  ecs::world->remove_system<spin_plugin>();
  ecs::world->remove_system<some_plugin>();
}

int some_plugin::foo() {
  count++;
  logger::core::Info("some_plugin::foo: {}", count);
  return 42;
}

void some_plugin::update(world*, ecs::component_view<const transform_component>) {

  count++;
  if (count > 180) {
    count = 0;
    logger::core::Info("some_plugin::update");
  }
}
