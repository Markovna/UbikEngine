#include "spin_plugin.h"
#include "core/plugins_registry.h"
#include "base/log.h"
#include "core/engine.h"
#include "core/engine_i.h"
#include "core/input_system.h"
#include "core/world.h"

class spin_plugin : public plugin<engine_i> {
 public:
  static int ver;
  int count = 0;
  int counter = 0;

  spin_plugin() = default;

  void update(engine*) override {
    count++;


    if (count > 180) {
      logger::core::Info("spin_plugin::update (ver.{}) {}", ver, counter++);
      count = 0;
    }
  }

  void start(engine*) override {}
  void stop(engine*) override {}

  ~spin_plugin() {
    logger::core::Info("~spin_plugin (ver.{})", ver);
  }
};

int spin_plugin::ver = 13;

void load_spin_plugin(engine *engine) {
  logger::core::Info("load_spin_plugin {}", spin_plugin::ver);

  spin_plugin *plugin = engine->plugins->add<spin_plugin>("spin_plugin");
  plugin->counter = 200;

  engine->plugins->add<some_plugin>("some_plugin");

}

void unload_spin_plugin(engine *engine) {
  logger::core::Info("unload_spin_plugin {}", spin_plugin::ver);

  spin_plugin* plugin = engine->plugins->get<spin_plugin>("spin_plugin");

  engine->plugins->remove("spin_plugin");
  engine->plugins->remove("some_plugin");

}

int some_plugin::foo() {
  count++;
  logger::core::Info("some_plugin::foo: {}", count);
  return 42;
}

void some_plugin::start(engine *e) { logger::core::Info("some_plugin::start {}", count); }

void some_plugin::update(engine *e) {

  count++;
  if (count > 180) {
    count = 0;
    logger::core::Info("some_plugin::update");
  }
}
