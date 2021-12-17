#pragma once

#include "gfx/experimental/gfx.h"

struct mesh_component {
  vertexbuf_handle vb;
  indexbuf_handle ib;
  uniform_handle uniform;
  uniformbuf_handle model_buffer;
  uniformbuf_handle camera_buffer;
};

void load_mesh(struct systems_registry& registry);


