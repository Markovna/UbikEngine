#pragma once

#include "core/plugins_registry.h"
#include "core/engine_i.h"

struct plugins_registry;

class some_plugin : public plugin<engine_i> {
 public:
  int count = 100;
  void update(engine *e) override;
  void start(engine *e) override;
  void stop(engine *e) override {}

  int foo();
};

extern "C" void load_spin_plugin(engine*);
extern "C" void unload_spin_plugin(engine*);
