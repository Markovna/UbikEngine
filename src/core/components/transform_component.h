#pragma once

#include "base/math.h"
#include "core/render_pipeline.h"

struct transform_component {
  transform local;
  mutable transform world;
  mutable bool dirty = false;
};

void register_transform_component(struct systems_registry& registry);




