#pragma once

#include "gfx_config.h"
#include "base/math.h"
#include "base/color.h"

#include <string>
#include <memory>

namespace gfx {

enum class handle_type {
  VertexBuffer = 0,
  IndexBuffer,
  FrameBuffer,
  Shader,
  Uniform,
  Texture
};

struct attribute {

  enum type {
    Float = 0,
    Int,
    Uint,
    Byte
  };

  static int get_size(type);

  struct binding {
    enum type : uint8_t {
      Position  = 0,
      Normal    = 1,
      Color0    = 2,
      Color1    = 3,
      Tangent   = 4,
      TexCoord0 = 5,
      TexCoord1 = 6,
      TexCoord2 = 7,
      TexCoord3 = 8,
      TexCoord4 = 9,
      TexCoord5 = 10,

      Count
    };

    static bool try_parse(std::string_view str, type& type);
  };

  struct binding_pack {
    uint16_t mask = 0;
    uint16_t locations[binding::Count] = {};
  };

  static binding_pack pack(std::initializer_list<binding::type> list) {
    binding_pack p;
    uint16_t location = 0;
    for (binding::type bnd : list) {
      p.mask |= (1u << bnd);
      p.locations[bnd] = location++;
    }
    return p;
  }

  static binding_pack& set_pack(binding_pack& pack, binding::type bnd, int location) {
    pack.mask |= (1u << bnd);
    pack.locations[bnd] = location;
    return pack;
  }

  struct format {
    static format Byte();
    static format Int();
    static format Uint();
    static format Float();
    static format Float2();
    static format Float3();
    static format Float4();
    static format Byte4();

    type type;
    uint8_t size;
  };

  binding::type binding = binding::Position;
  format format = format::Float();
  bool normalized = false;
};

class vertex_layout {
 public:
  struct item {
    attribute attribute;
    uint32_t offset = 0;
  };

  using iterator = const item*;

 public:
  vertex_layout() = default;
  ~vertex_layout() = default;
  vertex_layout(std::initializer_list<attribute> attributes);
  const item &operator[](uint32_t idx) const;

  [[nodiscard]] iterator begin() const { return std::begin(attributes_); }
  [[nodiscard]] iterator end() const { return std::begin(attributes_) + size_; }

  [[nodiscard]] uint32_t get_stride() const { return stride_; }
  [[nodiscard]] uint32_t get_size() const { return size_; }

 private:
  item attributes_[static_config::kAttributesCapacity] = {};
  uint32_t stride_ = 0;
  uint32_t size_ = 0;
};

template<class T>
struct handle_traits;

template<>
struct handle_traits<uint32_t> {
  static constexpr uint32_t index_mask       = 0x000FFFFFu;
  static constexpr uint8_t  generation_offs  = 20u;
  static constexpr uint32_t generation_mask  = 0xFFFu << generation_offs;
};

template<handle_type>
struct handle {
 public:
  using type = uint32_t;
  type id = traits::index_mask;

 private:
  using traits = handle_traits<type>;

 public:
  static handle& invalid() {
    static handle inst { traits::index_mask };
    return inst;
  }

  explicit constexpr operator bool() const noexcept {
    return *this != invalid();
  }

  bool operator==(handle other) const noexcept {
    return id == other.id;
  }

  bool operator!=(handle other) const noexcept {
    return !(*this == other);
  }

  [[nodiscard]] type index() const { return id & traits::index_mask; }
};

using vertexbuf_handle = handle<handle_type::VertexBuffer>;
using indexbuf_handle = handle<handle_type::IndexBuffer>;
using framebuf_handle = handle<handle_type::FrameBuffer>;
using shader_handle = handle<handle_type::Shader>;
using uniform_handle = handle<handle_type::Uniform>;
using texture_handle = handle<handle_type::Texture>;

using viewid_t = uint32_t;
using tex_slot = uint32_t;

struct texture_format {

  enum type {
    R8      = 0,
    RGB8,
    RGBA8,

    D24S8,

    Count
  };
};

struct texture_wrap {
  enum type : uint8_t {
    Repeat = 0,
    Mirrored,
    Clamp,
    ClampToBorder
  };

  type u = Repeat;
  type v = Repeat;
  type w = Repeat;
};

struct texture_filter {
  enum type : uint8_t {
    Linear = 0,
    Nearest
  };

  type min = Linear;
  type mag = Linear;
  type map = Linear;
};

struct texture_flags {
  using mask = uint32_t;

  enum type {
    None            = 0,
    RenderTarget    = 1u << 0u
  };
};

class buffer_ptr {
 public:
  buffer_ptr(std::shared_ptr<void> ptr, uint32_t size) noexcept : ptr_(std::move(ptr)), size_(size) {}

  buffer_ptr() = default;
  ~buffer_ptr() = default;

  buffer_ptr(buffer_ptr&&) noexcept = default;
  buffer_ptr(const buffer_ptr&) noexcept = default;

  buffer_ptr& operator=(buffer_ptr&& other) noexcept {
    buffer_ptr(std::move(other)).swap(*this);
    return *this;
  }

  buffer_ptr& operator=(const buffer_ptr& other) noexcept {
    buffer_ptr(other).swap(*this);
    return *this;
  }

  void reset() {
    ptr_.reset();
    size_ = 0;
  }

  [[nodiscard]] uint32_t size() const { return size_; }
  [[nodiscard]] void* get() const { return ptr_.get(); }

  explicit operator bool() const noexcept { return get(); }

 private:
  void swap(buffer_ptr& other) noexcept {
    std::swap(ptr_, other.ptr_);
    std::swap(size_, other.size_);
  }
 private:
  std::shared_ptr<void> ptr_ = nullptr;
  uint32_t size_ = 0;
};

struct config {
  void* window_handle = nullptr;
  vec2i resolution = {};
};

struct options {
  enum flag {
    None,
    DepthTest = 1u << 1u
  };
  using flags = uint32_t;
};

struct clear_flag {
  enum flag {
    Depth   = 1u << 1u,
    Color   = 1u << 2u,
    Stencil = 1u << 3u
  };

  using flags = uint32_t;
};

void init(const config&);
void shutdown();

void resolution(const vec2i&);
vec2i resolution();

vertexbuf_handle create_vertex_buffer(buffer_ptr ptr, uint32_t size, vertex_layout layout);
indexbuf_handle create_index_buffer(buffer_ptr ptr, uint32_t size);
framebuf_handle create_frame_buffer(std::initializer_list<texture_handle>);
uniform_handle create_uniform(const char* name);
shader_handle create_shader(const char* vertex_src, const char* fragment_src, attribute::binding_pack bindings);
texture_handle create_texture(uint32_t width, uint32_t height, texture_format::type, buffer_ptr ref = {});
texture_handle create_texture(uint32_t width, uint32_t height, texture_format::type, texture_wrap wrap, texture_filter filter, texture_flags::mask, buffer_ptr ptr);

void destroy(vertexbuf_handle&);
void destroy(indexbuf_handle&);
void destroy(framebuf_handle&);
void destroy(shader_handle&);
void destroy(uniform_handle&);
void destroy(texture_handle&);

viewid_t reserve_view();
void release_view(viewid_t);

void set_uniform(uniform_handle, int32_t);
void set_uniform(uniform_handle, uint32_t);
void set_uniform(uniform_handle, uint64_t);
void set_uniform(uniform_handle, bool);
void set_uniform(uniform_handle, float);
void set_uniform(uniform_handle, const vec3&);
void set_uniform(uniform_handle, const vec4&);
void set_uniform(uniform_handle, const color&);
void set_uniform(uniform_handle, const mat4&);
void set_uniform(uniform_handle, texture_handle, tex_slot);

void set_buffer(vertexbuf_handle, uint32_t offset = 0, uint32_t num = 0);
void set_buffer(indexbuf_handle, uint32_t offset = 0, uint32_t num = 0);

void set_transform(const mat4&);
void set_scissor(vec4i rect);
void set_options(options::flags);

void update_vertex_buffer(vertexbuf_handle, buffer_ptr ptr, uint32_t offset = 0);
void update_index_buffer(indexbuf_handle, buffer_ptr ptr, uint32_t offset = 0);

void set_view_rect(viewid_t, const vec4i& rect);
void set_view(viewid_t, const mat4&);
void set_projection(viewid_t, const mat4&);
void set_view_buffer(viewid_t, framebuf_handle);
void set_clear(viewid_t, clear_flag::flags);
void set_clear_color(viewid_t, const color&);

buffer_ptr copy(const void*, uint32_t);
buffer_ptr make_ref(void*, uint32_t);

void render(viewid_t, shader_handle);
void frame();

};


