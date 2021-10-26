#pragma once

#include "gfx.h"
#include "base/math.h"
#include "base/color.h"
#include "base/iterator_range.h"

#include <variant>

namespace gfx::details {

struct frame_command;
struct renderer_api;

void create_index_buffer_execute(frame_command *frame_command, renderer_api *api);
void create_vertex_buffer_execute(frame_command *frame_command, renderer_api *api);
void create_frame_buffer_execute(frame_command *frame_command, renderer_api *api);
void create_uniform_execute(frame_command *frame_command, renderer_api *api);
void update_index_buffer_execute(frame_command *frame_command, renderer_api *api);
void update_vertex_buffer_execute(frame_command *frame_command, renderer_api *api);
void create_texture_execute(frame_command *frame_command, renderer_api *api);
void create_shader_execute(frame_command *frame_command, renderer_api *api);
void destroy_index_buffer_execute(frame_command *cmd, renderer_api *api);
void destroy_vertex_buffer_execute(frame_command *cmd, renderer_api *api);
void destroy_frame_buffer_execute(frame_command *cmd, renderer_api *api);
void destroy_texture_execute(frame_command *cmd, renderer_api *api);
void destroy_uniform_execute(frame_command *cmd, renderer_api *api);
void destroy_shader_execute(frame_command *cmd, renderer_api *api);

using execute_func = void (*)(frame_command *, renderer_api *);
std::vector<execute_func> &frame_commands_execute_table();

renderer_api* CreateRendererApi(const config&);
void DestroyRendererApi(renderer_api* api);

struct uniform {
  enum type {
    Int  = 0,
    Uint,
    Uint2,
    Bool,
    Float,
    Vec3,
    Vec4,
    Color,
    Matrix
  };

  union {
    int32_t int_val = {};
    uint32_t uint_val;
    uint64_t uint2_val;
    bool bool_val;
    float float_val;
    vec3 vec3_val;
    vec4 vec4_val;
    color color_val;
    mat4 matrix_val;
  };

  type type = type::Int;
};

template<class T>
void set_uniform_value(details::uniform& uniform, T value);

template<>
void set_uniform_value<int32_t>(details::uniform& uniform, int32_t value);
template<>
void set_uniform_value<uint32_t>(details::uniform& uniform, uint32_t value);
template<>
void set_uniform_value<uint64_t>(details::uniform& uniform, uint64_t value);
template<>
void set_uniform_value<bool>(details::uniform& uniform, bool value);
template<>
void set_uniform_value<float>(details::uniform& uniform, float value);
template<>
void set_uniform_value<vec3>(details::uniform& uniform, vec3 value);
template<>
void set_uniform_value<vec4>(details::uniform& uniform, vec4 value);
template<>
void set_uniform_value<color>(details::uniform& uniform, color value);
template<>
void set_uniform_value<mat4>(details::uniform& uniform, mat4 value);

struct uniform_pair {
  uniform_handle handle;
  uniform value;
};

struct texture_format_info {
  size_t channel_size;
  uint8_t channels;
  uint8_t depth_bits;
  uint8_t stencil_bits;
};

const texture_format_info* get_texture_formats();

struct shader_type {
  enum type {
    Vertex = 0,
    Fragment = 1,

    Count
  };

  static bool try_parse(std::string_view str, type& type);
};

struct view {
  mat4 view = {};
  mat4 projection = {};
  vec4i viewport = {};
  clear_flag::flags clear_flags = 0;
  color clear_color = {};
  framebuf_handle frame_buffer = {};
};

struct draw_unit {
  viewid_t viewid = 0;
  mat4 transform = mat4::identity();
  indexbuf_handle ib_handle = {};
  vertexbuf_handle vb_handle = {};
  shader_handle shader_handle = {};
  uint32_t vb_offset = 0;
  uint32_t vb_size = 0;
  uint32_t ib_offset = 0;
  uint32_t ib_size = 0;
  vec4i scissor = {0,0,0,0};
  options::flags options = options::DepthTest;
  uniform_pair uniforms[static_config::kMaxUniformsPerDrawCall] = {};
  size_t uniforms_size = 0;
  texture_handle textures[static_config::kTextureSlotsCapacity] = {};
  uint32_t texture_slots[static_config::kTextureSlotsCapacity] = {};
  size_t texture_slots_size = 0;
};

struct create_index_buffer_command {
  indexbuf_handle handle = {};
  buffer_ptr ptr = {};
  uint32_t size = 0;
};

struct create_vertex_buffer_command {
  vertexbuf_handle handle = {};
  buffer_ptr ptr = {};
  uint32_t size = 0;
  vertex_layout layout = {};
};

struct create_frame_buffer_command {
  framebuf_handle handle = {};
  texture_handle handles[static_config::kFrameBufferMaxAttachments] = {};
  uint32_t size = 0;
  bool destroy_tex = false;
};

struct create_uniform_command {
  uniform_handle handle = {};
  char name[64];
};

struct update_index_buffer_command {
  indexbuf_handle handle = {};
  buffer_ptr ptr = {};
  uint32_t offset = 0;
};

struct update_vertex_buffer_command {
  vertexbuf_handle handle = {};
  buffer_ptr ptr = {};
  uint32_t offset = 0;
};

struct create_texture_command {
  texture_handle handle = {};
  buffer_ptr ptr = {};
  uint32_t width = 0;
  uint32_t height = 0;
  texture_format::type format = texture_format::RGBA8;
  texture_wrap wrap;
  texture_filter filter;
  texture_flags::mask flags;
};

struct create_shader_command {
  shader_handle handle;
  std::string vertex_src;
  std::string fragment_src;
  attribute::binding_pack bindings;
};

struct destroy_index_buffer_command {
  indexbuf_handle handle;
};

struct destroy_frame_buffer_command {
  framebuf_handle handle;
};

struct destroy_vertex_buffer_command {
  vertexbuf_handle handle;

};

struct destroy_texture_command {
  texture_handle handle;
};

struct destroy_uniform_command {
  uniform_handle handle;
};

struct destroy_shader_command {
  shader_handle handle;
};

struct frame_command : public std::variant<
    create_index_buffer_command,
    create_vertex_buffer_command,
    create_frame_buffer_command,
    create_texture_command,
    create_uniform_command,
    create_shader_command,
    update_vertex_buffer_command,
    update_index_buffer_command,
    destroy_index_buffer_command,
    destroy_vertex_buffer_command,
    destroy_frame_buffer_command,
    destroy_texture_command,
    destroy_uniform_command,
    destroy_shader_command
    > {
};

class frame {
 public:
  draw_unit& get_draw();

  void next();

  void reset();

  template <class T>
  T& emplace_command() {
    command_buffer_[commands_count_++].emplace<T>();
    return std::get<T>(command_buffer_[commands_count_-1]);
  }

  void set_views(const view *src);

  [[nodiscard]] const view &get_view(viewid_t id) const {
    return views_[id];
  }

  iterator_range<frame_command *> get_commands() {
    return {command_buffer_, command_buffer_ + commands_count_};
  }

  [[nodiscard]] iterator_range<const frame_command *> get_commands() const {
    return {command_buffer_, command_buffer_ + commands_count_};
  }

  [[nodiscard]] iterator_range<const draw_unit *> get_draws() const {
    return {draws_, draws_ + draws_count_};
  }

  [[nodiscard]] iterator_range<const view*> views() const {
    return {views_, views_ + static_config::kViewsCapacity};
  }

  [[nodiscard]] vec2i get_resolution() const { return resolution_; }
  void set_resolution(vec2i value) { resolution_ = value; }

 private:
  view views_[static_config::kViewsCapacity];
  draw_unit draws_[static_config::kMaxDrawCallsCount];
  uint32_t draws_count_ = 0;
  frame_command command_buffer_[static_config::kMaxFrameCommandsCount];
  uint32_t commands_count_ = 0;
  vec2i resolution_ = {};
};

template<class T, size_t Capacity>
class handle_pool {
 private:
  using type = typename T::type;
  using traits = handle_traits<type>;

  constexpr static const type null = traits::index_mask;
 public:
  T get() {
    type idx{};
    if (free_ == null) {
      idx = size_;
      store_[size_++] = idx;
    } else {
      auto free = store_[free_];
      store_[free_] = idx = free_ | (free & traits::generation_mask);
      free_ = free & traits::index_mask;
    }

    return T{idx};
  }

  void erase(T value) {
    auto index = value.index();
//    auto generation = ((value.id >> traits::generation_offs) + 1) << traits::generation_offs;
    auto generation = (value.id & traits::generation_mask) + (1 << traits::generation_offs);
    store_[index] = free_ | generation;
    free_ = index;
  }

  bool valid(T value) const {
    auto index = value.index();
    return index < Capacity && index < size_ && store_[index] == value.id;
  }

 private:
  type store_[Capacity] = {};
  type size_ = 0;
  type free_ = null;
};

}


