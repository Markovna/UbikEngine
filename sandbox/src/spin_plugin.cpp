#include "spin_plugin.h"

#include "base/math.h"
#include "base/log.h"

#include "core/plugins.h"
#include "core/input_system.h"
#include "core/world.h"
#include "core/meta/registration.h"

class spin_plugin : public world_system {
 public:
  static int ver;
  int count = 0;
  int counter = 0;

  spin_plugin() = default;

  void update(world*) override {
    count++;

    if (count > 180) {
      logger::core::Info("spin_plugin::update (ver.{}) {}", ver, counter++);
      count = 0;
    }
  }

  void start(world*) override {}
  void stop(world*) override {}

  ~spin_plugin() {
    logger::core::Info("~spin_plugin (ver.{})", ver);
  }
};

int spin_plugin::ver = 15;

void load_spin_plugin(plugins* plugins_registry) {

  logger::core::Info("load_spin_plugin {}", spin_plugin::ver);

  ecs::world->register_system<spin_plugin>("spin_plugin");
  ecs::world->register_system<some_plugin>("some_plugin");
}

void unload_spin_plugin(plugins* plugins_registry) {
  logger::core::Info("unload_spin_plugin {}", spin_plugin::ver);

  ecs::world->unregister_system("spin_plugin");
  ecs::world->unregister_system("some_plugin");
}

int some_plugin::foo() {
  count++;
  logger::core::Info("some_plugin::foo: {}", count);
  return 42;
}

void some_plugin::start(world*) { logger::core::Info("some_plugin::start {}", count); }

void some_plugin::update(world*) {

  count++;
  if (count > 180) {
    count = 0;
    logger::core::Info("some_plugin::update");
  }
}
