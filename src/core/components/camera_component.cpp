#include "camera_component.h"

int_set<uint32_t, gfx::static_config::kCamerasCapacity> &camera_component::registry() {
  static int_set<uint32_t, gfx::static_config::kCamerasCapacity> idx_registry{};
  return idx_registry;
}

void serialization<camera_component>::to_asset(asset& asset, const camera_component* comp) {
  asset["fov"] = comp->fov;
  asset["near"] = comp->near;
  asset["far"] = comp->far;
  asset["orthogonal_size"] = comp->orthogonal_size;

  auto& normalized_rect = asset["normalized_rect"];
  normalized_rect["x"] = comp->normalized_rect.x;
  normalized_rect["y"] = comp->normalized_rect.y;
  normalized_rect["z"] = comp->normalized_rect.z;
  normalized_rect["w"] = comp->normalized_rect.w;

  auto& col = asset["color"];
  col["r"] = comp->clear_color.r;
  col["g"] = comp->clear_color.g;
  col["b"] = comp->clear_color.b;
  col["a"] = comp->clear_color.a;

  asset["clear_flags"] = comp->clear_flags;
}

void serialization<camera_component>::from_asset(const asset& asset, camera_component *comp) {
  comp->fov = asset["fov"];
  comp->near = asset["near"];
  comp->far = asset["far"];
  comp->orthogonal_size = asset["orthogonal_size"];

  auto& normalized_rect = asset["normalized_rect"];
  comp->normalized_rect.x = normalized_rect["x"];
  comp->normalized_rect.y = normalized_rect["y"];
  comp->normalized_rect.z = normalized_rect["z"];
  comp->normalized_rect.w = normalized_rect["w"];

  auto& col = asset["color"];
  comp->clear_color.r = col["r"];
  comp->clear_color.g = col["g"];
  comp->clear_color.b = col["b"];
  comp->clear_color.a = col["a"];

  comp->clear_flags = asset["clear_flags"];
}
