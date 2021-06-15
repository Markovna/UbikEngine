#include "camera_component.h"

int_set<uint32_t, gfx::static_config::kCamerasCapacity> &camera_component::registry() {
  static int_set<uint32_t, gfx::static_config::kCamerasCapacity> idx_registry{};
  return idx_registry;
}

void serialization<camera_component>::to_asset(asset& asset, const camera_component& comp) {
  assets::set(asset, "fov", comp.fov);
  assets::set(asset, "near", comp.near);
  assets::set(asset, "far", comp.far);
  assets::set(asset, "orthogonal_size", comp.orthogonal_size);
  assets::set(asset, "normalized_rect", comp.normalized_rect);
  assets::set(asset, "clear_flags", comp.clear_flags);
}

void serialization<camera_component>::from_asset(const asset& asset, camera_component& comp) {
  assets::get(asset, "fov", comp.fov);
  assets::get(asset, "near", comp.near);
  assets::get(asset, "far", comp.far);
  assets::get(asset, "orthogonal_size", comp.orthogonal_size);
  assets::get(asset, "normalized_rect", comp.normalized_rect);
  assets::get(asset, "clear_flags", comp.clear_flags);
}
