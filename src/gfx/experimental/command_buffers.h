#pragma once

#include "gfx.h"

#include <memory>

class allocator;

struct command_base {
  virtual ~command_base() = default;
};

template<class Type>
struct command_header {
  Type type;
  std::unique_ptr<command_base> ptr;
};

template<class T, class F>
static T& get_command(const command_header<F>& header) {
  assert(T::command_type == header.type);
  return *static_cast<T*>(header.ptr.get());
}

template<class T>
class command_buffer {
 public:
  template<class Command>
  void add(Command&& command) {
    using type = std::remove_reference_t<Command>;
    headers_.push_back({ .type = type::command_type, .ptr = std::make_unique<type>(std::forward<Command>(command))});
  }

  template<class Command, class ...Args>
  Command& emplace(Args&& ...args) {
    auto& header = headers_.emplace_back();
    header.type = Command::command_type;
    header.ptr = std::make_unique<Command>(std::forward<Args>(args)...);
    return *static_cast<Command*>(header.ptr.get());
  }

  void clear() {
    headers_.resize(0);
  }

  template<class Compare>
  void sort(Compare compare) {
    std::stable_sort(headers_.begin(), headers_.end(), compare);
  }

  [[nodiscard]] auto begin() const { return headers_.begin(); }
  [[nodiscard]] auto end() const { return headers_.end(); }

 private:
  std::vector<command_header<T>> headers_;
};

enum class render_command_type {
  BIND_RENDER_PASS,
  SET_VIEWPORT,
  SET_SCISSOR,
  DRAW,
};

struct render_command_base : public command_base {
  uint32_t sort_key;
};

template<render_command_type Type>
struct render_command : public render_command_base {
  static constexpr auto command_type = Type;
};

struct bind_render_pass_command : public render_command<render_command_type::BIND_RENDER_PASS> {
  framebuf_handle handle;
  bool depth_test;
};

struct set_viewport_command : public render_command<render_command_type::SET_VIEWPORT> {
  vec4i viewport;
};


struct set_scissor_command : public render_command<render_command_type::SET_SCISSOR> {
  vec4i rect;
};

struct uniform_bindings {

};

struct draw_command : public render_command<render_command_type::DRAW> {
  vertexbuf_handle vb_handle;
  indexbuf_handle ib_handle;
  shader_handle shader;
  uniform_handle uniforms[MAX_BINDINGS_PER_DRAW];
  uint32_t uniforms_count;
  uint32_t vertex_offset;
  uint32_t index_offset;
  uint32_t size;
};

struct draw_call_desc {
  uint32_t sort_key = 0;
  shader_handle shader = { };
  vertexbuf_handle vertexbuf = { };
  indexbuf_handle indexbuf = { };
  uint32_t size = 0;
  uint32_t vb_offset = 0;
  uint32_t ib_offset = 0;
  std::initializer_list<uniform_handle> uniforms = { };
};

enum class resource_command_type {
  CREATE_VERTEX_BUFFER,
  CREATE_INDEX_BUFFER,
  CREATE_UNIFORM_BUFFER,
  CREATE_FRAME_BUFFER,
  CREATE_TEXTURE,
  CREATE_UNIFORM,
  CREATE_SHADER,
  UPDATE_VERTEX_BUFFER,
  UPDATE_INDEX_BUFFER,
  UPDATE_UNIFORM_BUFFER,
  SET_UNIFORM,
  DESTROY_VERTEX_BUFFER,
  DESTROY_INDEX_BUFFER,
  DESTROY_UNIFORM_BUFFER,
  DESTROY_FRAME_BUFFER,
  DESTROY_TEXTURE,
  DESTROY_UNIFORM,
  DESTROY_SHADER,
};

template<resource_command_type Type>
struct resource_command : public command_base {
  static constexpr auto command_type = Type;
};

struct create_vertex_buffer_command : public resource_command<resource_command_type::CREATE_VERTEX_BUFFER> {
  vertexbuf_handle handle = {};
  memory memory;
  vertex_layout layout = {};
};

struct update_vertex_buffer_command : public resource_command<resource_command_type::UPDATE_VERTEX_BUFFER> {
  vertexbuf_handle handle = {};
  memory memory;
  uint32_t offset;
};

struct update_index_buffer_command : public resource_command<resource_command_type::UPDATE_INDEX_BUFFER> {
  indexbuf_handle handle = {};
  memory memory;
  uint32_t offset;
};

struct update_uniform_buffer_command : public resource_command<resource_command_type::UPDATE_UNIFORM_BUFFER> {
  uniformbuf_handle handle = {};
  memory memory;
  uint32_t offset;
};

struct create_index_buffer_command : public resource_command<resource_command_type::CREATE_INDEX_BUFFER> {
  indexbuf_handle handle = {};
  memory memory;
};

struct create_uniform_buffer_command : public resource_command<resource_command_type::CREATE_UNIFORM_BUFFER> {
  uniformbuf_handle handle = {};
  memory memory;
};

struct create_texture_command : public resource_command<resource_command_type::CREATE_TEXTURE> {
  texture_handle handle;
  texture_desc desc;
  memory memory;
};

struct create_frame_buffer_command : public resource_command<resource_command_type::CREATE_FRAME_BUFFER> {
  framebuf_handle handle;
  texture_handle attachments[MAX_FRAMEBUFFER_ATTACHMENTS];
  uint32_t attachments_size;
};

struct destroy_frame_buffer_command : public resource_command<resource_command_type::DESTROY_FRAME_BUFFER> {
  framebuf_handle handle;
};

struct destroy_vertex_buffer_command : public resource_command<resource_command_type::DESTROY_VERTEX_BUFFER> {
  vertexbuf_handle handle;
};

struct destroy_index_buffer_command : public resource_command<resource_command_type::DESTROY_INDEX_BUFFER> {
  indexbuf_handle handle;
};

struct destroy_uniform_buffer_command : public resource_command<resource_command_type::DESTROY_UNIFORM_BUFFER> {
  uniformbuf_handle handle;
};

struct destroy_texture_command : public resource_command<resource_command_type::DESTROY_TEXTURE> {
  texture_handle handle;
};

struct destroy_uniform_command : public resource_command<resource_command_type::DESTROY_UNIFORM> {
  uniform_handle handle;
};

struct create_uniform_command : public resource_command<resource_command_type::CREATE_UNIFORM> {
  uniform_handle handle;
  uniform_desc uniforms[MAX_UNIFORM_BINDINGS];
  uint32_t uniforms_size;
};

static constexpr uint32_t MAX_UNIFORM_BUFFER_SIZE = 1024;

struct set_uniform_command : public resource_command<resource_command_type::SET_UNIFORM> {
  set_uniform_command(
      uniform_handle _handle,
      uint32_t _binding,
      texture_handle _texture)
  : handle(_handle), binding(_binding), type(uniform_type::type::SAMPLER), texture(_texture)
  {}

  set_uniform_command(
      uniform_handle _handle,
      uint32_t _binding,
      uniformbuf_handle _buffer)
  : handle(_handle), binding(_binding), type(uniform_type::type::BUFFER), buffer(_buffer)
  {}

  uniform_handle handle = {};
  uint32_t binding = 0;
  uniform_type::type type = uniform_type::type::SAMPLER;
  union {
    texture_handle texture = {};
    uniformbuf_handle buffer;
  };
};

struct create_shader_command : public resource_command<resource_command_type::CREATE_SHADER> {
  shader_handle handle;
  shader_compile_result vertex;
  shader_compile_result fragment;
};

struct destroy_shader_command : public resource_command<resource_command_type::DESTROY_SHADER> {
  shader_handle handle;
};

class resource_command_buffer : command_buffer<resource_command_type> {
 private:
  using base = command_buffer<resource_command_type>;

 public:
  resource_command_buffer(handle_allocator_set* ha, allocator* a)
    : base(),
      handle_allocators_(ha),
      allocator_(a),
      queued_free_(),
      memory_()
  {}

  vertexbuf_handle create_vertex_buffer(const class vertex_layout_desc&, size_t);
  vertexbuf_handle create_vertex_buffer(const class vertex_layout_desc&, size_t, memory&);
  void update_vertex_buffer(vertexbuf_handle, uint32_t size, memory&, uint32_t offset = 0);
  void destroy_vertex_buffer(vertexbuf_handle);

  indexbuf_handle create_index_buffer(size_t);
  indexbuf_handle create_index_buffer(size_t, memory&);
  void update_index_buffer(indexbuf_handle, uint32_t size, memory&, uint32_t offset = 0);
  void destroy_index_buffer(indexbuf_handle);

  uniformbuf_handle create_uniform_buffer(size_t);
  uniformbuf_handle create_uniform_buffer(size_t, memory&);
  void update_uniform_buffer(uniformbuf_handle, uint32_t size, memory&, uint32_t offset = 0);
  void destroy_uniform_buffer(uniformbuf_handle);

  framebuf_handle create_frame_buffer(std::initializer_list<texture_handle>);
  void destroy_frame_buffer(framebuf_handle);

  shader_handle create_shader(shader_program_desc);
  void destroy_shader(shader_handle);

  texture_handle create_texture(texture_desc);
  texture_handle create_texture(texture_desc, memory&);
  void destroy_texture(texture_handle);

  uniform_handle create_uniform(std::initializer_list<uniform_desc> list);
  void set_uniform(uniform_handle handle, uint32_t binding, uniformbuf_handle);
  void set_uniform(uniform_handle handle, uint32_t binding, texture_handle);
  void destroy_uniform(uniform_handle);

  [[nodiscard]] auto begin() const { return base::begin(); }
  [[nodiscard]] auto end() const { return base::end(); }

  void free_resources();

 private:
  template<class T>
  inline static void free_handle(uint32_t id, handle_allocator_set* allocator) {
    allocator->free( T { id } );
  }

  template<class HandleType>
  inline HandleType alloc() {
    return handle_allocators_->alloc<HandleType>();;
  }

  template<class HandleType>
  inline void free(HandleType& handle) { return handle_allocators_->free(handle); }

  inline void alloc(size_t size, memory&);

  template<class HandleType>
  inline void queue_free_handle(HandleType& handle) {
    queued_free_.emplace_back(handle.id, &free_handle<HandleType>);
  }

  inline void queue_free_memory(const memory& memory);

 private:
  using free_handle_fn_t = void(*)(uint32_t, handle_allocator_set*);

  handle_allocator_set* handle_allocators_;
  allocator* allocator_;
  std::vector<std::pair<uint32_t, free_handle_fn_t>> queued_free_;
  std::vector<memory> memory_;
};

class render_command_buffer : command_buffer<render_command_type> {
 private:
  using base = command_buffer<render_command_type>;

 public:
  void bind_render_pass(uint32_t sort_key, framebuf_handle fb_handle, bool depth_test = true);
  void set_viewport(uint32_t sort_key, vec4i);
  void set_scissor(uint32_t sort_key, vec4i);
  void draw(draw_call_desc);

  void sort();

  auto begin() const { return base::begin(); }
  auto end() const { return base::end(); }
};
