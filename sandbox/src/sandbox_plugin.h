#pragma once

#include "core/serialization.h"
#include "core/components/component.h"

struct inner_type {
  double a;
};

class custom_component : public component<custom_component> {
 public:
  int a;
 private:
  float b;
  inner_type c;

  friend serialization<custom_component>;
};

struct engine;

extern "C" void load_sandbox_plugin(engine*);
extern "C" void unload_sandbox_plugin(engine*);

template<>
struct serialization<custom_component> {
  void from_asset(const asset &asset, custom_component* comp);
  void to_asset(asset &asset, const custom_component* comp);
};
