#pragma once

#include "gfx/gfx.h"
#include "assets/texture.h"

class render_texture {
 public:
  render_texture(
        uint32_t width,
        uint32_t height,
        gfx::texture_format::type,
        gfx::texture_wrap = {},
        gfx::texture_filter = {},
        gfx::texture_flags::mask = gfx::texture_flags::None);

  ~render_texture();

  [[nodiscard]] const texture& texture() const { return color_texture_; }
  [[nodiscard]] gfx::framebuf_handle handle() const { return fb_handle_; }

private:
  class texture color_texture_;
  gfx::texture_handle depth_tex_handle_;
  gfx::framebuf_handle fb_handle_;
};
