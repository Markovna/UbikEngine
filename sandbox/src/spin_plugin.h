#pragma once

#include "core/world.h"

struct test_component {
  int a;
};

struct plugins;

class some_plugin : public ecs::system {
 public:
  int count = 100;
  void update(world*, ecs::component_view<const transform_component>);

  int foo();
};

extern "C" void load_spin_plugin(plugins*);
extern "C" void unload_spin_plugin(plugins*);
