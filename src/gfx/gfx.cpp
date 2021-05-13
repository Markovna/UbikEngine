#include "gfx.h"
#include "gfx_details.h"
#include "renderer_api.h"
#include "base/iterator_range.h"

#include <vector>
#include <unordered_map>
#include <variant>
#include <assert.h>

const gfx::vertex_layout::item &gfx::vertex_layout::operator[](uint32_t idx) const {
  assert(idx >= 0 && idx < size_);
  return attributes_[idx];
}

struct gfx::attribute::format gfx::attribute::format::Byte() {
  return { gfx::attribute::type::Byte, 1 };
}

struct gfx::attribute::format gfx::attribute::format::Int() {
  return { gfx::attribute::type::Int, 1 };
}

struct gfx::attribute::format gfx::attribute::format::Uint() {
  return { gfx::attribute::type::Uint, 1 };
}

struct gfx::attribute::format gfx::attribute::format::Float() {
  return { gfx::attribute::type::Float, 1 };
}

struct gfx::attribute::format gfx::attribute::format::Vec4Byte() {
  return { gfx::attribute::type::Byte, 4 };
}

struct gfx::attribute::format gfx::attribute::format::Vec4() {
  return { gfx::attribute::type::Float, 4 };
}

struct gfx::attribute::format gfx::attribute::format::Vec3() {
  return { gfx::attribute::type::Float, 3 };
}

struct gfx::attribute::format gfx::attribute::format::Vec2() {
  return { gfx::attribute::type::Float, 2 };
}

gfx::vertex_layout::vertex_layout(std::initializer_list<attribute> attributes) {
  assert(attributes.size() <= static_config::kAttributesCapacity);

  for (const auto& attr : attributes) {
    attributes_[size_].attribute = attr;
    attributes_[size_].offset = stride_;
    stride_ += attr.format.size * attribute::get_size(attr.format.type);
    size_++;
  }
}

int gfx::attribute::get_size(gfx::attribute::type type) {
  switch (type) {
    case gfx::attribute::type::Float: return sizeof(float);
    case gfx::attribute::type::Uint: return sizeof(uint32_t);
    case gfx::attribute::type::Int: return sizeof(int32_t);
    case gfx::attribute::type::Byte: return sizeof(int8_t);
  }
  return 0;
}

namespace gfx::details {

struct state {
  handle_pool<indexbuf_handle, static_config::kIndexBuffersCapacity> index_buffer_handles;
  handle_pool<vertexbuf_handle, static_config::kVertexBuffersCapacity> vertex_buffer_handles;
  handle_pool<framebuf_handle, static_config::kFrameBuffersCapacity> frame_buffer_handles;
  handle_pool<shader_handle, static_config::kShadersCapacity> shader_handles;
  handle_pool<texture_handle, static_config::kTexturesCapacity> texture_handles;
  handle_pool<uniform_handle, static_config::kUniformsCapacity> uniform_handles;

  camera cameras[static_config::kCamerasCapacity] = {};
  struct frame frame = {};
  renderer_api* api = nullptr;
};

config g_config;
state g_state;

}

bool gfx::attribute::binding::try_parse(std::string_view str, gfx::attribute::binding::type &type) {
#define DEFINE_BINDING(__binding) { #__binding, __binding }
  static std::unordered_map<std::string, attribute::binding::type> types_map = {
      DEFINE_BINDING(Position),
      DEFINE_BINDING(Normal),
      DEFINE_BINDING(Color0),
      DEFINE_BINDING(Color1),
      DEFINE_BINDING(Tangent),
      DEFINE_BINDING(TexCoord0),
      DEFINE_BINDING(TexCoord1),
      DEFINE_BINDING(TexCoord2),
      DEFINE_BINDING(TexCoord3),
      DEFINE_BINDING(TexCoord4),
      DEFINE_BINDING(TexCoord5)
  };
#undef DEFINE_BINDING

  static std::string search_str;
  search_str = str;
  if (auto it = types_map.find(search_str); it != types_map.end()) {
    type = it->second;
    return true;
  }

  return false;
}

namespace gfx {

void init(const config& config) {
  details::g_config = config;
  assert(!details::g_state.api);
  details::g_state.api = details::CreateRendererApi(config);
}

vertexbuf_handle create_vertex_buffer(buffer_ptr ptr, uint32_t size, vertex_layout layout) {
  vertexbuf_handle handle(details::g_state.vertex_buffer_handles.get());
  auto& command = details::g_state.frame.emplace_command<details::create_vertex_buffer_command>();
  command.handle = handle;
  command.ptr = std::move(ptr);
  command.size = size;
  command.layout = layout;
  return handle;
}

indexbuf_handle create_index_buffer(buffer_ptr ptr, uint32_t size) {
  indexbuf_handle handle(details::g_state.index_buffer_handles.get());
  auto& command = details::g_state.frame.emplace_command<details::create_index_buffer_command>();
  command.handle = handle;
  command.ptr = std::move(ptr);
  command.size = size;
  return handle;
}

framebuf_handle create_frame_buffer(std::initializer_list<texture_handle> textures) {
  assert(textures.size() < static_config::kFrameBufferMaxAttachments);
  framebuf_handle handle(details::g_state.frame_buffer_handles.get());
  auto& command = details::g_state.frame.emplace_command<details::create_frame_buffer_command>();
  command.handle = handle;
  command.destroy_tex = false;
  for (auto tex : textures)
    command.handles[command.size++] = tex;

  return handle;
}

void shutdown() {
  assert(details::g_state.api);
  details::DestroyRendererApi(details::g_state.api);
}

void frame() {
  for (details::frame_command& command : details::g_state.frame.get_commands()) {
    std::vector<details::execute_func>& table = details::frame_commands_execute_table();
    table[command.index()](&command, details::g_state.api);
  }

  static int frame_count = 0;
  frame_count++;

  details::g_state.frame.set_resolution(details::g_config.resolution);
  details::g_state.frame.set_cameras(details::g_state.cameras);

  details::g_state.api->RenderFrame(details::g_state.frame);

  details::g_state.frame.reset();
}

void render(camera_id camera_id, shader_handle handle) {
  details::draw_unit &draw = details::g_state.frame.get_draw();
  draw.camera_id = camera_id;
  draw.shader_handle = handle;
  details::g_state.frame.next();
}

void destroy(vertexbuf_handle& handle) {
  assert(handle && "Renderer::Destroy failed: Invalid handle");

  auto& command = details::g_state.frame.emplace_command<details::destroy_vertex_buffer_command>();
  command.handle = handle;
  details::g_state.vertex_buffer_handles.erase(handle);
  handle = {};
}

void destroy(indexbuf_handle& handle) {
  assert(handle && "Renderer::Destroy failed: Invalid handle");

  auto& command = details::g_state.frame.emplace_command<details::destroy_index_buffer_command>();
  command.handle = handle;
  details::g_state.index_buffer_handles.erase(handle);
  handle = {};
}

void destroy(framebuf_handle& handle) {
  assert(handle && "Renderer::Destroy failed: Invalid handle");

  auto& command = details::g_state.frame.emplace_command<details::destroy_frame_buffer_command>();
  command.handle = handle;
  details::g_state.frame_buffer_handles.erase(handle);
  handle = {};
}

void destroy(shader_handle& handle) {
  assert(handle && "Renderer::Destroy failed: Invalid handle");

  auto& command = details::g_state.frame.emplace_command<details::destroy_shader_command>();
  command.handle = handle;
  details::g_state.shader_handles.erase(handle);
  handle = {};
}

void destroy(uniform_handle& handle) {
  assert(handle && "Renderer::Destroy failed: Invalid handle");

  auto& command = details::g_state.frame.emplace_command<details::destroy_uniform_command>();
  command.handle = handle;
  details::g_state.uniform_handles.erase(handle);
  handle = {};
}

void destroy(texture_handle& handle) {
  assert(handle && "Renderer::Destroy failed: Invalid handle");

  auto& command = details::g_state.frame.emplace_command<details::destroy_texture_command>();
  command.handle = handle;
  details::g_state.texture_handles.erase(handle);
  handle = {};
}

template<class T>
void set_uniform_impl(uniform_handle handle, T value) {
  details::draw_unit &draw = details::g_state.frame.get_draw();
  auto& uniform = draw.uniforms[draw.uniforms_size++];
  uniform.handle = handle;
  details::set_uniform_value(uniform.value, value);
}

void set_uniform(uniform_handle handle, int value) {
  set_uniform_impl(handle, value);
}
void set_uniform(uniform_handle handle, float value) {
  set_uniform_impl(handle, value);
}
void set_uniform(uniform_handle handle, bool value) {
  set_uniform_impl(handle, value);
}
void set_uniform(uniform_handle handle, const vec3& value) {
  set_uniform_impl(handle, value);
}
void set_uniform(uniform_handle handle, const vec4& value) {
  set_uniform_impl(handle, value);
}
void set_uniform(uniform_handle handle, const color& value) {
  set_uniform_impl(handle, value);
}
void set_uniform(uniform_handle handle, const mat4& value) {
  set_uniform_impl(handle, value);
}
void set_uniform(uniform_handle handle, texture_handle tex_handle, tex_slot slot) {
  details::draw_unit &draw = details::g_state.frame.get_draw();
  draw.textures[slot] = tex_handle;
  draw.texture_slots[draw.texture_slots_size++] = slot;
  set_uniform(handle, (int) slot);
}

}