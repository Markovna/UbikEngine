#pragma once

struct color {
  static const color& White();
  static const color& Black();
  static const color& Red();
  static const color& Green();
  static const color& Blue();

  float r, g, b, a;
};


