#include "renderer.h"
#include "render_context.h"

namespace experimental::gfx {

swap_chain renderer::create_swap_chain(window::window_handle win_handle) {
  auto fb_handle = handle_allocators_.alloc<framebuf_handle>();
  auto sc_handle = handle_allocators_.alloc<swap_chain_handle>();
  context_->create_swap_chain(win_handle, sc_handle, fb_handle);
  return swap_chain { .handle = sc_handle, .back_buffer_handle = fb_handle};
}

void renderer::destroy_swap_chain(swap_chain &swap_chain) {
  context_->destroy_swap_chain(swap_chain);
  handle_allocators_.free(swap_chain.back_buffer_handle);
  handle_allocators_.free(swap_chain.handle);
}

void renderer::swap(swap_chain &swap_chain) {
  context_->swap(swap_chain);
}

void renderer::submit(resource_command_buffer* buffer) {
  context_->submit(buffer);
  buffer->free_resources();
  delete buffer;
}

void renderer::submit(const render_command_buffer* buffer) {
  context_->submit(buffer);
  delete buffer;
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

}