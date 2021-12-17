#pragma once

#include <base/math.h>
#include <base/color.h>

struct camera_component {
  float fov = 60.0f;
  float near = 0.1f;
  float far = 100.0f;
  float orthogonal_size = 1.0f;
  vec4 normalized_rect = {0.0f, 0.0f, 1.0f, 1.0f};
  color clear_color = color::black();
};