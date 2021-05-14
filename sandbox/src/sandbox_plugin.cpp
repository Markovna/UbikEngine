#include "sandbox_plugin.h"
#include "spin_plugin.h"
#include "core/engine.h"

class test : public plugin_base {
 public:
  int count = 0;
  void stop(engine *e) override {}
  void start(engine *e) override {}
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