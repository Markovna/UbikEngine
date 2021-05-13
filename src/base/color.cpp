#include "color.h"

const color& color::White() {
  static color col = { 1.0f, 1.0f, 1.0f, 1.0f };
  return col;
}
const color& color::Black() {
  static color col = { 0.0f, 0.0f, 0.0f, 1.0f };
  return col;
}
const color& color::Red() {
  static color col = { 1.0f, 0.0f, 0.0f, 1.0f };
  return col;
}
const color& color::Green() {
  static color col = { 0.0f, 1.0f, 0.0f, 1.0f };
  return col;
}
const color& color::Blue() {
  static color col = { 0.0f, 0.0f, 1.0f, 1.0f };
  return col;
}