#pragma once

#include "base/allocator.h"
#include "gfx/gfx.h"
#include "platform/window.h"
#include "gfx/command_buffers.h"

class render_context;

class renderer {
 public:
  using create_context_fn = std::unique_ptr<render_context>(*)();

  explicit renderer(create_context_fn create_context);

  renderer(const renderer&) = delete;
  renderer(renderer&&) = delete;

  renderer& operator=(const renderer&) = delete;
  renderer& operator=(renderer&&) = delete;

  std::unique_ptr<resource_command_buffer> create_resource_command_buffer() { return std::make_unique<resource_command_buffer>(&handle_allocators_, allocator_.get()); }
  std::unique_ptr<render_command_buffer> create_render_command_buffer() { return std::make_unique<render_command_buffer>(); }

  [[nodiscard]] std::string_view backend_name() const;

  void begin_frame();
  void submit(resource_command_buffer& buffer);
  void submit(render_command_buffer& buffer);

  swap_chain create_swap_chain(window::window_handle win_handle);
  void resize_swap_chain(const swap_chain&, vec2i size);
  void destroy_swap_chain(swap_chain &swap_chain);
  void swap(swap_chain &swap_chain);

 private:
  std::unique_ptr<render_context> context_;
  handle_allocator_set handle_allocators_;
  std::unique_ptr<allocator> allocator_;
};
