#include "render_texture.h"

render_texture::render_texture(
    uint32_t width,
    uint32_t height,
    gfx::texture_format::type format,
    gfx::texture_wrap wrap,
    gfx::texture_filter filter,
    gfx::texture_flags::mask flags)
    : color_texture_(nullptr, width, height, format, wrap, filter, flags)
    , depth_tex_handle_(gfx::create_texture(width, height, gfx::texture_format::D24S8, {}, {}, gfx::texture_flags::RenderTarget, {}))
    , fb_handle_(gfx::create_frame_buffer({color_texture_.handle(), depth_tex_handle_}))
{}

render_texture::~render_texture() {
    gfx::destroy(depth_tex_handle_);
    gfx::destroy(fb_handle_);
}
