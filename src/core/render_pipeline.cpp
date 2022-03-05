#include "render_pipeline.h"
#include "core/world.h"
#include "core/components/transform_component.h"
#include "core/meta/interface_registry.h"
#include "core/systems_registry.h"

static system_ptr<::interface_registry> g_interface_registry;

void render_pipeline::render(
   uint32_t sort_key,
   const viewer &viewer,
   renderer &renderer,
   render_command_buffer &render_cmd_buf,
   resource_command_buffer &resource_cmd_buf) {

  view view {
      .sort_key = sort_key,
      .camera = viewer.camera
  };

  render_cmd_buf.bind_render_pass(view.sort_key, viewer.color_target);

  if (viewer.size.x > 0 && viewer.size.y > 0) {
    render_cmd_buf.set_viewport(view.sort_key, {0, 0, viewer.size.x, viewer.size.y});
  }

  auto render_interface_view = g_interface_registry->get_interface_view<render_interface>();
  auto transform_pool = viewer.world->get_pool<transform_component>();

  resolve_transforms(*viewer.world, viewer.world->root());

  for (auto render_interface_id : render_interface_view) {
    auto& [id, render] = render_interface_view.get(render_interface_id);
    if (!viewer.world->has(id))
      continue;

    render(view, transform_pool, viewer.world->pool_base(id), renderer, render_cmd_buf, resource_cmd_buf);
  }
}

void init_render_pipeline(const systems_registry& registry) {
  g_interface_registry = registry.get<::interface_registry>();
}