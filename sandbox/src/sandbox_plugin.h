#pragma once

#include "core/serialization.h"
#include "core/components/component.h"

struct inner_type {
  double a;
};

class custom_component {
 public:
  int a;
 private:
  float b;
  inner_type c;

  friend serializer<custom_component>;
};

register_component(custom_component);

struct plugins;

extern "C" void load_sandbox_plugin(plugins*);
extern "C" void unload_sandbox_plugin(plugins*);

template<>
struct serializer<custom_component> {
  static void from_asset(const asset &asset, custom_component& comp);
  static void to_asset(asset &asset, const custom_component& comp);
};

template<>
struct serializer<inner_type> {
  static void from_asset(const asset &asset, inner_type& comp);
  static void to_asset(asset &asset, const inner_type& comp);
};
