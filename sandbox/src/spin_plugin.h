#pragma once

#include "core/plugins_registry.h"

struct plugins_registry;

class some_plugin : public plugin_base {
 public:
  int count = 100;
  void update(engine *e) override;
  void start(engine *e) override;
  void stop(engine *e) override {}

  int foo();
};

extern "C" void load_spin_plugin(plugins_registry* reg);
extern "C" void unload_spin_plugin(plugins_registry *reg);
