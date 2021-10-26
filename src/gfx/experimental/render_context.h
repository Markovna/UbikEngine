#pragma once

#include "gfx.h"
#include "platform/window.h"

namespace experimental::gfx {

class resource_command_buffer;
class render_command_buffer;

class render_context {
 public:
  render_context() = default;

  render_context(const render_context&) = delete;
  render_context(render_context&&) = delete;

  render_context& operator=(const render_context&) = delete;
  render_context& operator=(render_context&&) = delete;

  virtual void begin_frame() = 0;
  virtual void submit(const resource_command_buffer*) = 0;
  virtual void submit(const render_command_buffer*) = 0;
  virtual void create_swap_chain(window::window_handle,  swap_chain_handle, framebuf_handle) = 0;
  virtual void destroy_swap_chain(swap_chain& swap_chain) = 0;
  virtual void swap(swap_chain& swap_chain) = 0;
  virtual ~render_context() = default;
};

}

