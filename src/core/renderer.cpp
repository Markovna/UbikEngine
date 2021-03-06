#include "renderer.h"
#include "gfx/render_context.h"

swap_chain renderer::create_swap_chain(window::window_handle win_handle) {
  auto fb_handle = handle_allocators_.alloc<framebuf_handle>();
  auto sc_handle = handle_allocators_.alloc<swap_chain_handle>();
  context_->create_swap_chain(win_handle, sc_handle, fb_handle);
  return swap_chain { .handle = sc_handle, .back_buffer_handle = fb_handle};
}

void renderer::resize_swap_chain(const swap_chain& sc, vec2i size) {
  context_->resize_swap_chain(sc, size);
}

void renderer::destroy_swap_chain(swap_chain &swap_chain) {
  context_->destroy_swap_chain(swap_chain);
  handle_allocators_.free(swap_chain.back_buffer_handle);
  handle_allocators_.free(swap_chain.handle);
}

void renderer::swap(swap_chain &swap_chain) {
  context_->swap(swap_chain);
}

void renderer::submit(resource_command_buffer& buffer) {
  context_->submit(buffer);
}

void renderer::submit(render_command_buffer& buffer) {
  buffer.sort();
  context_->submit(buffer);
}

void renderer::begin_frame() {
  context_->begin_frame();
}

std::string_view renderer::backend_name() const {
  return context_->name();
}

renderer::renderer(renderer::create_context_fn create_context) :
    context_(create_context()),
    handle_allocators_(),
    allocator_(std::make_unique<allocator_default>())
{
  assert(context_);
}

