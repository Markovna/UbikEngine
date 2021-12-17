#pragma once

#include "core/world.h"
#include "gfx/experimental/gfx.h"
#include "gfx/experimental/renderer.h"
#include "gfx/experimental/camera_component.h"
#include "mesh_component.h"
#include "base/delegate.h"

struct camera_view_projection {
  mat4 view;
  mat4 projection;
};

struct view {
  uint32_t sort_key;
  camera_view_projection camera;
};

struct viewer {
  const world* world;
  const camera_component* camera;
  const transform_component* transform;
  framebuf_handle color_target;
  vec2 size;
};

struct render_i {
  const delegate<void(view&,
                const ecs::details::component_pool<transform_component>*,
                const ecs::details::component_pool_base*,
                renderer&,
                render_command_buffer&,
                resource_command_buffer&)> render;
};

class render_pipeline {
 public:

  void render(const viewer& viewer, renderer& renderer) {

    render_command_buffer* render_buffer = renderer.create_render_command_buffer();
    resource_command_buffer* resource_buffer = renderer.create_resource_command_buffer();

     float ratio = viewer.size.x / viewer.size.y;

    view view {
        .sort_key = 0,
        .camera = {
          .view = mat4::trs(viewer.transform->world.position, viewer.transform->world.rotation, viewer.transform->world.scale),
          .projection = mat4::perspective(
              viewer.camera->fov * math::DEG_TO_RAD,
              ratio * viewer.camera->normalized_rect.z / viewer.camera->normalized_rect.w,
              viewer.camera->near,
              viewer.camera->far
          )
        }
    };

    render_buffer->bind_render_pass(view.sort_key, viewer.color_target);

    for (auto& [id, render] : meta::get_interface_view<render_i>()) {
      if (!viewer.world->has(id))
        continue;

      render.render(view, viewer.world->get_pool<transform_component>(), viewer.world->pool_base(id), renderer, *render_buffer, *resource_buffer);
    }

    renderer.submit(resource_buffer);
    renderer.submit(render_buffer);
  }

 private:
};

