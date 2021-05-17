#pragma once

#include <cstdint>
#include "base/color.h"
#include "gfx/gfx.h"
#include "base/int_set.h"

struct camera_component {
 private:
  using clear_flag = gfx::clear_flag::flags;

 private:
  static int_set<uint32_t, gfx::static_config::kCamerasCapacity>& registry();

  static uint32_t next_id() {
    return registry().next();
  }

  static void free_id(uint32_t id) {
    registry().free(id);
  }

 public:
  enum kind_t { Game, Editor };

 public:
  explicit camera_component(kind_t kind = Game) : idx(next_id()), kind(kind) {}
  ~camera_component() { free_id(idx); }

  uint32_t idx;
  float fov = 60.0f;
  float near = 0.1f;
  float far = 100.0f;
  float orthogonal_size = 1.0f;
  vec4 normalized_rect = {0.0f, 0.0f, 1.0f, 1.0f};
  color clear_color = color::black();
  clear_flag clear_flags = gfx::clear_flag::Color | gfx::clear_flag::Depth;
  kind_t kind = Game;
};