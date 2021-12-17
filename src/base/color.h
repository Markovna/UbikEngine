#pragma once

struct color {
  static const color& white();
  static const color& black();
  static const color& red();
  static const color& green();
  static const color& blue();

  float r, g, b, a;
};


