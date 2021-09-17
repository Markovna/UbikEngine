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

void serializer<color>::from_asset(assets::provider* p, const asset& asset, color& color) {
  assets::get(p, asset, "r", color.r);
  assets::get(p, asset, "g", color.g);
  assets::get(p, asset, "b", color.b);
  assets::get(p, asset, "a", color.a);
}

void serializer<color>::to_asset(asset& asset, const color& color) {
  assets::set(asset, "r", color.r);
  assets::set(asset, "g", color.g);
  assets::set(asset, "b", color.b);
  assets::set(asset, "a", color.a);
}
