#pragma once

#include "core/serialization.h"

struct color {
  static const color& white();
  static const color& black();
  static const color& red();
  static const color& green();
  static const color& blue();

  float r, g, b, a;
};

template<>
struct serializer<color> {
  static void from_asset(assets::provider*, const asset&, color&);
  static void to_asset(asset&, const color&);
};


