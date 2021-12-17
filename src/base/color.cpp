#include "color.h"

const color& color::white() {
  static color col = { 1.0f, 1.0f, 1.0f, 1.0f };
  return col;
}
const color& color::black() {
  static color col = { 0.0f, 0.0f, 0.0f, 1.0f };
  return col;
}
const color& color::red() {
  static color col = { 1.0f, 0.0f, 0.0f, 1.0f };
  return col;
}
const color& color::green() {
  static color col = { 0.0f, 1.0f, 0.0f, 1.0f };
  return col;
}
const color& color::blue() {
  static color col = { 0.0f, 0.0f, 1.0f, 1.0f };
  return col;
}


