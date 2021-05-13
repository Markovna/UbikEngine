#include "renderer.h"
#include "world.h"
#include "gfx/gfx.h"

void renderer::render(world* world) {

  auto camera_view = world->view<camera_component, transform_component>();
  auto mesh_view = world->view<mesh_component, transform_component>();
  for (entity camera_ent : camera_view) {
    auto [camera, cam_transform] = camera_view.get<camera_component, transform_component>(camera_ent);
    for (entity mesh_ent : mesh_view) {

   }
  }
}
