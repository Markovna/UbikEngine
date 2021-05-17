#pragma once

#include "core/components/camera_component.h"

struct world;

class renderer {
 public:
  static void render(world* world, vec4 viewport, gfx::framebuf_handle fb_handle, camera_component::kind_t camera_kind);
};


