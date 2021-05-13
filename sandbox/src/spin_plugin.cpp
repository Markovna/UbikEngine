#include "spin_plugin.h"
#include "core/plugins_registry.h"
#include "base/log.h"
#include "core/engine.h"
#include "core/input_system.h"
#include "core/world.h"

class test_plugin {
 public:
  int count = 0;

  void update1(engine*) {
    count++;

    if (count > 180) {
      logger::core::Info("test_plugin::update");
      count = 0;
    }
  }
};

class spin_plugin {
 public:
  static int ver;
  int count = 0;
  int counter = 0;

  spin_plugin() = default;

  void update(engine*) {
    count++;

    if (count > 180) {
      logger::core::Info("spin_plugin::update (ver.{}) {}", ver, counter++);
      count = 0;
    }
  }

  void start(engine*) {}
  void stop(engine*) {}

  ~spin_plugin() {
    logger::core::Info("~spin_plugin (ver.{})", ver);
  }
};

int spin_plugin::ver = 10;

void load_spin_plugin(plugins_registry *reg) {
  logger::core::Info("load_spin_plugin {}", spin_plugin::ver);
  spin_plugin* plugin = reg->add_plugin<spin_plugin>("spin_plugin");
  plugin->counter = 200;

  reg->add_plugin<test_plugin>("test_plugin");
}

void unload_spin_plugin(plugins_registry *reg) {
  logger::core::Info("unload_spin_plugin {}", spin_plugin::ver);

  spin_plugin* plugin = reg->get_plugin<spin_plugin>("spin_plugin");

  reg->remove_plugin("spin_plugin");
  reg->remove_plugin("test_plugin");
}