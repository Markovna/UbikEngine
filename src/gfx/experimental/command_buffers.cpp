#include <base/allocator.h>
#include "command_buffers.h"
#include "vertex_layout_desc.h"

namespace experimental::gfx {

void render_command_buffer::bind_render_pass(uint32_t sort_key, framebuf_handle fb_handle) {
  auto &command = emplace<bind_render_pass_command>();
  command.handle = fb_handle;
}

void render_command_buffer::draw(draw_call_desc desc) {
  assert(desc.shader);
  assert(desc.vertexbuf);

  auto& command = emplace<draw_command>();
  command.shader = desc.shader;
  command.vb_handle = desc.vertexbuf;
  command.ib_handle = desc.indexbuf;
  command.index_offset = 0;
  command.vertex_offset = 0;
  command.size = 0;

  command.bindings.uniforms_count = desc.uniforms.size();
  uint32_t i = 0;
  for (auto& uniform : desc.uniforms) {
    command.bindings.uniforms[i++] = uniform;
  }
}

void render_command_buffer::set_viewport(uint32_t sort_key, vec4i viewport) {
  auto& command = emplace<set_viewport_command>();
  command.viewport = viewport;
}

vertexbuf_handle resource_command_buffer::create_vertex_buffer(const vertex_layout_desc& layout_desc, size_t size) {
  auto& command = emplace<create_vertex_buffer_command>();
  command.handle = alloc<vertexbuf_handle>();
  command.memory.data = nullptr;
  command.memory.size = size;
  command.layout = (vertex_layout) layout_desc;

  return command.handle;
}

void resource_command_buffer::destroy_vertex_buffer(vertexbuf_handle handle) {
  auto& command = emplace<destroy_vertex_buffer_command>();
  command.handle = handle;

  queue_free_handle(handle);
}

uniform_handle resource_command_buffer::create_uniform(std::initializer_list<uniform_desc> list) {
  auto& command = emplace<create_uniform_command>();
  command.handle = alloc<uniform_handle>();
  uint32_t i = 0;
  for (auto& desc : list) {
    command.uniforms[i++] = desc;
  }
  command.uniforms_size = list.size();
  return command.handle;
}

void resource_command_buffer::update_uniform(uniform_handle handle, uint32_t binding, texture_handle texture) {
  auto& command = emplace<update_uniform_command>();
  command.handle = handle;
  command.binding = binding;
  command.texture = texture;
}

void resource_command_buffer::update_uniform(uniform_handle handle, uint32_t binding, void* buffer, uint32_t size) {
  auto& command = emplace<update_uniform_command>();
  command.handle = handle;
  command.binding = binding;
  command.size = size;
  std::memcpy(command.buffer, buffer, size);
}

void resource_command_buffer::free_resources() {
  for (auto& [id, fn] : queued_free_) {
    fn(id, handle_allocators_);
  }

  for (auto& mem : memory_) {
    ::free(*allocator_, reinterpret_cast<void*>(mem.data));
  }
}

static void copy_shader_blob(memory& dst, shader_blob& src, allocator* allocator) {
  dst.data = reinterpret_cast<uint8_t*>(::alloc(*allocator, src.size));
  dst.size = src.size;
  std::memcpy(dst.data, src.data, src.size);
}

shader_handle resource_command_buffer::create_shader(shader_program_desc desc) {
  auto& command = emplace<create_shader_command>();
  command.handle = alloc<shader_handle>();

  copy_shader_blob(command.vertex, desc.vertex_shader, allocator_);
  copy_shader_blob(command.fragment, desc.fragment_shader, allocator_);

  queue_free_memory(command.vertex);
  queue_free_memory(command.fragment);

  return command.handle;
}

void resource_command_buffer::destroy_uniform(uniform_handle handle) {
  auto& command = emplace<destroy_uniform_command>();
  command.handle = handle;

  queue_free_handle(handle);
}

void resource_command_buffer::destroy_shader(shader_handle handle) {
  auto& command = emplace<destroy_shader_command>();
  command.handle = handle;

  queue_free_handle(handle);
}

indexbuf_handle resource_command_buffer::create_index_buffer(size_t size) {
  auto& command = emplace<create_index_buffer_command>();
  command.handle = alloc<indexbuf_handle>();
  command.memory.data = nullptr;
  command.memory.size = size;
  return command.handle;
}

framebuf_handle resource_command_buffer::create_frame_buffer(std::initializer_list<texture_handle> attachments) {
  auto& command = emplace<create_frame_buffer_command>();
  command.handle = alloc<framebuf_handle>();
  uint32_t i = 0;
  for (auto& a : attachments) {
    command.attachments[i++] = a;
  }
  command.attachments_size = attachments.size();
  return command.handle;
}

void resource_command_buffer::destroy_frame_buffer(framebuf_handle handle) {
  auto& command = emplace<destroy_frame_buffer_command>();
  command.handle = handle;
}

void resource_command_buffer::destroy_index_buffer(indexbuf_handle handle) {
  auto& command = emplace<destroy_index_buffer_command>();
  command.handle = handle;

  queue_free_handle(handle);
}

void resource_command_buffer::destroy_texture(texture_handle handle) {
  auto& command = emplace<destroy_texture_command>();
  command.handle = handle;

  queue_free_handle(handle);
}

vertexbuf_handle resource_command_buffer::create_vertex_buffer(const vertex_layout_desc& desc, size_t size, memory& mem) {
  auto& command = emplace<create_vertex_buffer_command>();
  command.handle = alloc<vertexbuf_handle>();
  command.layout = (vertex_layout) desc;
  alloc(size, mem);
  command.memory = mem;

  queue_free_memory(mem);

  return command.handle;
}

void resource_command_buffer::queue_free_memory(const memory& memory) {
  memory_.push_back(memory);
}

void resource_command_buffer::update_vertex_buffer(vertexbuf_handle handle, uint32_t size, memory& mem, uint32_t offset) {
  auto& command = emplace<update_vertex_buffer_command>();
  command.handle = handle;
  alloc(size, mem);
  command.offset = offset;
  command.memory = mem;

  queue_free_memory(mem);
}

indexbuf_handle resource_command_buffer::create_index_buffer(size_t size, memory& mem) {
  auto& command = emplace<create_index_buffer_command>();
  command.handle = alloc<indexbuf_handle>();
  alloc(size, mem);
  command.memory = mem;

  queue_free_memory(mem);
  return command.handle;
}

void resource_command_buffer::update_index_buffer(indexbuf_handle handle, uint32_t size, memory& mem, uint32_t offset) {
  auto& command = emplace<update_index_buffer_command>();
  command.handle = handle;
  alloc(size, mem);
  command.offset = offset;
  command.memory = mem;

  queue_free_memory(mem);
}

texture_handle resource_command_buffer::create_texture(texture_desc desc) {
  auto& command = emplace<create_texture_command>();
  command.handle = alloc<texture_handle>();
  command.desc = std::move(desc);

  uint32_t size =
      command.desc.width * command.desc.height *
      texture_format::info[command.desc.format].channel_size *
      texture_format::info[command.desc.format].channels;

  command.memory.data = nullptr;
  command.memory.size = size;

  return command.handle;
}

texture_handle resource_command_buffer::create_texture(texture_desc desc, memory& mem) {
  auto& command = emplace<create_texture_command>();
  command.handle = alloc<texture_handle>();
  command.desc = std::move(desc);

  uint32_t size = command.desc.width * command.desc.height *
                texture_format::info[command.desc.format].channel_size *
                texture_format::info[command.desc.format].channels;

  alloc(size, mem);
  command.memory = mem;
  queue_free_memory(mem);
  return command.handle;
}

void resource_command_buffer::alloc(size_t size, memory& mem) {
  mem.data = reinterpret_cast<uint8_t*>(::alloc(*allocator_, size));
  mem.size = size;
}

}