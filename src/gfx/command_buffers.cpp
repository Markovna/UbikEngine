#include <base/allocator.h>
#include "command_buffers.h"
#include "gfx/vertex_layout_desc.h"

void render_command_buffer::bind_render_pass(uint32_t sort_key, framebuf_handle fb_handle, bool depth_test) {
  auto &command = emplace<bind_render_pass_command>();
  command.sort_key = sort_key;
  command.handle = fb_handle;
  command.depth_test = depth_test;
}

void render_command_buffer::draw(draw_call_desc desc) {
  assert(desc.shader);
  assert(desc.vertexbuf);

  auto& command = emplace<draw_command>();
  command.sort_key = desc.sort_key;
  command.shader = desc.shader;
  command.vb_handle = desc.vertexbuf;
  command.ib_handle = desc.indexbuf;
  command.index_offset = desc.ib_offset;
  command.vertex_offset = desc.vb_offset;
  command.size = desc.size;

  command.uniforms_count = desc.uniforms.size();
  uint32_t i = 0;
  for (auto& uniform : desc.uniforms) {
    command.uniforms[i++] = uniform;
  }
}

void render_command_buffer::set_scissor(uint32_t sort_key, vec4i rect) {
  auto& command = emplace<set_scissor_command>();
  command.sort_key = sort_key;
  command.rect = rect;
}

void render_command_buffer::set_viewport(uint32_t sort_key, vec4i viewport) {
  auto& command = emplace<set_viewport_command>();
  command.sort_key = sort_key;
  command.viewport = viewport;
}

void render_command_buffer::sort() {
  base::sort([](const command_header<render_command_type>& lhs, const command_header<render_command_type>& rhs) {
    auto* cmd0 = static_cast<const render_command_base*>(lhs.ptr.get());
    auto* cmd1 = static_cast<const render_command_base*>(rhs.ptr.get());
    return cmd0->sort_key < cmd1->sort_key;
  });
}

vertexbuf_handle resource_command_buffer::create_vertex_buffer(const vertex_layout_desc& layout_desc, size_t size) {
  auto& command = emplace<create_vertex_buffer_command>();
  command.handle = alloc<vertexbuf_handle>();
  command.memory.data = nullptr;
  command.memory.size = size;
  command.layout = (vertex_layout) layout_desc;

  return command.handle;
}

void resource_command_buffer::destroy_vertex_buffer(vertexbuf_handle& handle) {
  auto& command = emplace<destroy_vertex_buffer_command>();
  command.handle = handle;

  queue_free_handle(handle);

  handle = { };
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

void resource_command_buffer::set_uniform(uniform_handle handle, uint32_t binding, texture_handle texture) {
  emplace<set_uniform_command>(handle, binding, texture);
}

void resource_command_buffer::set_uniform(uniform_handle handle, uint32_t binding, uniformbuf_handle buffer) {
  emplace<set_uniform_command>(handle, binding, buffer);
}

void resource_command_buffer::free_resources() {
  for (auto& [id, fn] : queued_free_) {
    fn(id, handle_allocators_);
  }

  for (auto& mem : memory_) {
    ::free(*allocator_, reinterpret_cast<void*>(mem.data));
  }
}

shader_handle resource_command_buffer::create_shader(shader_program_desc desc) {
  auto& command = emplace<create_shader_command>();
  command.handle = alloc<shader_handle>();
  command.vertex = std::move(desc.vertex);
  command.fragment = std::move(desc.fragment);

  return command.handle;
}

void resource_command_buffer::destroy_uniform(uniform_handle& handle) {
  auto& command = emplace<destroy_uniform_command>();
  command.handle = handle;

  queue_free_handle(handle);

  handle = { };
}

void resource_command_buffer::destroy_shader(shader_handle& handle) {
  auto& command = emplace<destroy_shader_command>();
  command.handle = handle;

  queue_free_handle(handle);

  handle = { };
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

void resource_command_buffer::destroy_frame_buffer(framebuf_handle& handle) {
  auto& command = emplace<destroy_frame_buffer_command>();
  command.handle = handle;

  queue_free_handle(handle);

  handle = { };
}

void resource_command_buffer::destroy_index_buffer(indexbuf_handle& handle) {
  auto& command = emplace<destroy_index_buffer_command>();
  command.handle = handle;

  queue_free_handle(handle);

  handle = { };
}

uniformbuf_handle resource_command_buffer::create_uniform_buffer(size_t size) {
  auto& command = emplace<create_uniform_buffer_command>();
  command.handle = alloc<uniformbuf_handle>();
  command.memory.data = nullptr;
  command.memory.size = size;
  return command.handle;
}

uniformbuf_handle resource_command_buffer::create_uniform_buffer(size_t size, memory& mem) {
  auto& command = emplace<create_uniform_buffer_command>();
  command.handle = alloc<uniformbuf_handle>();
  alloc(size, mem);
  command.memory = mem;

  queue_free_memory(mem);
  return command.handle;
}

void resource_command_buffer::update_uniform_buffer(uniformbuf_handle handle, uint32_t size, memory& mem, uint32_t offset) {
  auto& command = emplace<update_uniform_buffer_command>();
  command.handle = handle;
  alloc(size, mem);
  command.offset = offset;
  command.memory = mem;

  queue_free_memory(mem);
}

void resource_command_buffer::destroy_uniform_buffer(uniformbuf_handle& handle) {
  auto& command = emplace<destroy_uniform_buffer_command>();
  command.handle = handle;

  queue_free_handle(handle);

  handle = { };
}

void resource_command_buffer::destroy_texture(texture_handle& handle) {
  auto& command = emplace<destroy_texture_command>();
  command.handle = handle;

  queue_free_handle(handle);

  handle = { };
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
