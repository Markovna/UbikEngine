#pragma once

#include "base/allocator.h"
#include "gfx.h"
#include "platform/window.h"
#include "command_buffers.h"

namespace experimental::gfx {

class render_context;

class renderer {
 public:
  using create_context_fn = std::unique_ptr<render_context>(*)();

  explicit renderer(create_context_fn create_context);

  renderer(const renderer&) = delete;
  renderer(renderer&&) = delete;

  renderer& operator=(const renderer&) = delete;
  renderer& operator=(renderer&&) = delete;

  resource_command_buffer* create_resource_command_buffer() { return new resource_command_buffer(&handle_allocators_, allocator_.get()); }
  render_command_buffer* create_render_command_buffer() { return new render_command_buffer(); }

  [[nodiscard]] std::string_view backend_name() const;

  void begin_frame();
  void submit(resource_command_buffer* buffer);
  void submit(const render_command_buffer* buffer);

  swap_chain create_swap_chain(window::window_handle win_handle);
  void destroy_swap_chain(swap_chain &swap_chain);
  void swap(swap_chain &swap_chain);

 private:

  std::unique_ptr<render_context> context_;
  handle_allocator_set handle_allocators_;
  std::unique_ptr<allocator> allocator_;
};

}