#pragma once

#include "base/math.h"
#include "gfx/gfx.h"

struct view_projection {
  mat4 view;
  mat4 projection;
};

struct viewer {
  const struct world* world;
  view_projection camera;
  framebuf_handle color_target = { framebuf_handle::invalid };
  vec2i size = { 0, 0 };
};