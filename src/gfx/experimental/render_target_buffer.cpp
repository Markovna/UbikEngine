#include "render_target_buffer.h"
#include "gfx/experimental/command_buffers.h"

render_target create_render_target(resource_command_buffer& res_cmd_buf, vec2i size) {
  render_target render_target;
  render_target.color_target = res_cmd_buf.create_texture({
                                                              .width  = (uint32_t) size.x,
                                                              .height = (uint32_t) size.y,
                                                              .format = texture_format::RGBA8
                                                          });

  render_target.depth_target = res_cmd_buf.create_texture({
                                                              .width  = (uint32_t) size.x,
                                                              .height = (uint32_t) size.y,
                                                              .format = texture_format::D24S8,
                                                              .flags = { texture_flag::RENDER_TARGET }
                                                          });

  render_target.fb_handle = res_cmd_buf.create_frame_buffer({ render_target.color_target, render_target.depth_target });
  return render_target;
}