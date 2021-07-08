#pragma once

#include "base/color.h"
#include "base/intset.h"
#include "gfx/gfx.h"
#include "core/components/component.h"
#include "core/serialization.h"

#include <cstdint>

struct camera_component : public component<camera_component> {
 private:
  using clear_flag = gfx::clear_flag::flags;

 public:
  struct tag_t {
    enum _enum {
      None = 0,
      Game = 1u << 1u,
      Editor = 1u << 2u
    };
    using type = uint32_t;
  };

 public:
  explicit camera_component() : viewid(gfx::reserve_view()) {}
  ~camera_component() { gfx::release_view(viewid); }

  gfx::viewid_t viewid;
  float fov = 60.0f;
  float near = 0.1f;
  float far = 100.0f;
  float orthogonal_size = 1.0f;
  vec4 normalized_rect = {0.0f, 0.0f, 1.0f, 1.0f};
  color clear_color = color::black();
  clear_flag clear_flags = gfx::clear_flag::Color | gfx::clear_flag::Depth;
  tag_t::type tag;
};

template<>
struct serializer<camera_component> {
  static void from_asset(const asset&, camera_component&);
  static void to_asset(asset&, const camera_component&);
};