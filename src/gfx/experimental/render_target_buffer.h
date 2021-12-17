#pragma once

#include "gfx/experimental/gfx.h"

struct resource_command_buffer;

struct render_target {
  framebuf_handle fb_handle;
  texture_handle color_target;
  texture_handle depth_target;
};

render_target create_render_target(resource_command_buffer&, vec2i);


