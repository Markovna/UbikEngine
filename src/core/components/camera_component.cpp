#include "camera_component.h"

int_set<uint32_t, gfx::static_config::kCamerasCapacity> &camera_component::registry() {
  static int_set<uint32_t, gfx::static_config::kCamerasCapacity> idx_registry{};
  return idx_registry;
}