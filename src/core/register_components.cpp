#include "register_components.h"
#include "core/components/mesh_component.h"
#include "core/components/transform_component.h"
#include "core/components/camera_component.h"

void register_components(struct systems_registry& reg) {
  register_transform_component(reg);
  register_camera_component(reg);
  register_mesh_component(reg);
}