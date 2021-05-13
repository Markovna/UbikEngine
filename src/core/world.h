#pragma once

#include "ecs.h"

using entity = ecs::entity;

struct transform_component {

};

struct mesh_component {

};

struct camera_component {
  uint32_t idx;
  float fov = 60.0f;
  float near = 0.1f;
  float far = 100.0f;
  float orthogonal_size = 1.0f;
};

class world : public ecs::registry {

};


