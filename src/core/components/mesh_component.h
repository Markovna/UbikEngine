#pragma once

#include "gfx/gfx.h"

struct mesh_component {
  vertexbuf_handle vb;
  indexbuf_handle ib;
  uniform_handle uniform;
  uniformbuf_handle model_buffer;
  uniformbuf_handle camera_buffer;
};

void register_mesh_component(struct systems_registry& registry);


