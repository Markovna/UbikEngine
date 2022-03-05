#include "viewport.h"


void viewport::create(resource_command_buffer &cmd_buf) {
  assert(!texture_uniform_);

  texture_uniform_ = cmd_buf.create_uniform({{.type = uniform_type::SAMPLER, .binding = 0}});
  color_target_ =
      cmd_buf.create_texture({
         .width  = (uint32_t) size_.x,
         .height = (uint32_t) size_.y,
         .format = texture_format::RGBA8
     });

  depth_target_ =
      cmd_buf.create_texture({
                                 .width  = (uint32_t) size_.x,
         .height = (uint32_t) size_.y,
         .format = texture_format::D24S8,
         .flags = { texture_flag::RENDER_TARGET }
     });

  framebuf_ = cmd_buf.create_frame_buffer(
      {color_target_, depth_target_ }
  );

  cmd_buf.set_uniform(texture_uniform_, 0, color_target_);
}

void viewport::free_resources(resource_command_buffer &cmd_buf) {
  if (texture_uniform_) {
    cmd_buf.destroy_texture(color_target_);
    cmd_buf.destroy_texture(depth_target_);
    cmd_buf.destroy_frame_buffer(framebuf_);
    cmd_buf.destroy_uniform(texture_uniform_);
  }
}

viewport::~viewport() {
  clear();
}

void viewport::clear() {
  auto cmd_buf = renderer_->create_resource_command_buffer();
  free_resources(*cmd_buf);
  renderer_->submit(*cmd_buf);

  size_ = { 0, 0 };
}

void viewport::resize(vec2i size) {
  if (size_ == size)
    return;

  size_ = size;

  auto cmd_buf = renderer_->create_resource_command_buffer();
  free_resources(*cmd_buf);
  create(*cmd_buf);
  renderer_->submit(*cmd_buf);
}
