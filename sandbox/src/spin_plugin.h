#pragma once

#include "core/world.h"

struct test_component {
  int a;
};

struct plugins;

class some_plugin : public world_system {
 public:
  int count = 100;
  void update(world*) override;
  void start(world*) override;
  void stop(world*) override {}

  int foo();
};

extern "C" void load_spin_plugin(plugins*);
extern "C" void unload_spin_plugin(plugins*);
