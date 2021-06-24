#pragma once

#include "base/color.h"
#include "base/int_set.h"
#include "gfx/gfx.h"
#include "core/components/component.h"
#include "core/serialization.h"

#include <cstdint>

struct camera_component : public component<camera_component> {
 private:
  using clear_flag = gfx::clear_flag::flags;

 private:
  static int_set<uint32_t, gfx::static_config::kCamerasCapacity>& registry();

  static uint32_t next_id() {
    return registry().alloc();
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

template<>
struct serializer<camera_component> {
  static void from_asset(const asset&, camera_component&);
  static void to_asset(asset&, const camera_component&);
};