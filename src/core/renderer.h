#pragma once

#include "core/components/camera_component.h"

struct world;
struct entity;

class renderer {
 public:
  using camera_predicate_t = std::function<bool(entity, const camera_component&)>;

 public:
  static void update_views(world* world, vec4 viewport, gfx::framebuf_handle fb_handle, const camera_predicate_t& predicate = {});
  static void render(world* world, const camera_predicate_t& predicate = {});
  static void render(
      const mat4& model,
      gfx::shader_handle shader,
      gfx::vertexbuf_handle vb,
      gfx::indexbuf_handle ib,
      const camera_component& camera);
};

