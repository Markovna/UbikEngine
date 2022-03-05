#pragma once

#include <cstdint>
#include <iostream>
#include <vector>
#include <base/memory.h>
#include <base/flags.h>

#include "base/math.h"

template<uint32_t>
struct handle {
  static constexpr uint32_t invalid = 0xFFFFFFFFu;

  uint32_t id = invalid;

  explicit constexpr operator bool() const noexcept {
    return id != invalid;
  }

  bool operator==(handle other) const noexcept {
    return id == other.id;
  }

  bool operator!=(handle other) const noexcept {
    return !(*this == other);
  }
};

enum handle_type : uint32_t {
  VERTEX_BUFFER_HANDLE_TYPE,
  INDEX_BUFFER_HANDLE_TYPE,
  UNIFORM_BUFFER_HANDLE_TYPE,
  FRAME_BUFFER_HANDLE_TYPE,
  SHADER_HANDLE_TYPE,
  TEXTURE_HANDLE_TYPE,
  UNIFORM_HANDLE_TYPE,
  SWAP_CHAIN_HANDLE_TYPE,
};

using vertexbuf_handle = handle<VERTEX_BUFFER_HANDLE_TYPE>;
using indexbuf_handle = handle<INDEX_BUFFER_HANDLE_TYPE>;
using uniformbuf_handle = handle<UNIFORM_BUFFER_HANDLE_TYPE>;
using framebuf_handle = handle<FRAME_BUFFER_HANDLE_TYPE>;
using shader_handle = handle<SHADER_HANDLE_TYPE>;
using texture_handle = handle<TEXTURE_HANDLE_TYPE>;
using uniform_handle = handle<UNIFORM_HANDLE_TYPE>;
using swap_chain_handle = handle<SWAP_CHAIN_HANDLE_TYPE>;

struct swap_chain {
  swap_chain_handle handle;
  framebuf_handle back_buffer_handle;
};

struct texture_wrap {
  enum type : uint8_t {
    REPEAT = 0,
    MIRRORED,
    CLAMP,
    CLAMP_TO_BORDER
  };

  type u = REPEAT;
  type v = REPEAT;
  type w = REPEAT;
};

struct uniform_type {
  enum type : uint8_t {
    BUFFER = 0,
    SAMPLER,

    COUNT
  };
};

struct texture_format_info {
  uint8_t channel_size;
  uint8_t channels;
  uint8_t depth_bits;
  uint8_t stencil_bits;
};

struct texture_format {
  enum type {
    R8      = 0,
    RGB8,
    RGBA8,

    D24S8,

    COUNT
  };

  constexpr static const texture_format_info info[] = {
      { sizeof(uint8_t),    1,   0,   0 }, // R8
      { sizeof(uint8_t),    3,   0,   0 }, // RGB8
      { sizeof(uint8_t),    4,   0,   0 }, // RGBA8
      { 0,    0,   24,  8 }, // D24S8
  };

  static_assert(sizeof(info) == COUNT * sizeof(texture_format_info), "Numbers of types and infos don't match");
};

struct texture_filter {
  enum type : uint8_t {
    LINEAR = 0,
    NEAREST
  };

  type min = LINEAR;
  type mag = LINEAR;
  type map = LINEAR;
};

enum class texture_flag{
  RENDER_TARGET,

  COUNT
};

using texture_flags = flags<texture_flag>;

struct texture_desc {
  uint32_t width;
  uint32_t height;
  texture_format::type format;
  texture_wrap wrap;
  texture_filter filter;
  texture_flags flags;
};

struct vertex_semantic {
  enum type {
    POSITION = 0,
    NORMAL,
    COLOR0,
    COLOR1,
    COLOR2,
    COLOR3,
    TANGENT,
    TEXCOORD0,
    TEXCOORD1,
    TEXCOORD2,
    TEXCOORD3,
    TEXCOORD4,
    TEXCOORD5,

    COUNT
  };

#define TYPE_NAME(__type) "_"#__type
  constexpr static const char* const names[] = {
    TYPE_NAME(POSITION),
    TYPE_NAME(NORMAL),
    TYPE_NAME(COLOR0),
    TYPE_NAME(COLOR1),
    TYPE_NAME(COLOR2),
    TYPE_NAME(COLOR3),
    TYPE_NAME(TANGENT),
    TYPE_NAME(TEXCOORD0),
    TYPE_NAME(TEXCOORD1),
    TYPE_NAME(TEXCOORD2),
    TYPE_NAME(TEXCOORD3),
    TYPE_NAME(TEXCOORD4),
    TYPE_NAME(TEXCOORD5),
  };
#undef TYPE_NAME

  static_assert(sizeof(names) == COUNT * sizeof(char*), "Numbers of types and names don't match");
};

struct vertex_type {
  enum type {
    FLOAT = 0,
    INT8,
    INT16,
    INT32,
    UINT8,
    UINT16,
    UINT32,

    COUNT
  };

  constexpr static const uint32_t sizes[] = {
    sizeof(float),
    sizeof(int8_t),
    sizeof(int16_t),
    sizeof(int32_t),
    sizeof(uint8_t),
    sizeof(uint16_t),
    sizeof(uint32_t),
  };

  static_assert(sizeof(sizes) == COUNT * sizeof(uint32_t), "Numbers of types and sizes don't match");
};

struct vertex_layout {
  struct item {
    uint32_t offset;
    vertex_type::type type;
    uint32_t size;
    bool normalized;
  };
  item items[vertex_semantic::COUNT];
  uint32_t stride;
};

static constexpr uint32_t MAX_UNIFORM_BINDINGS = 64;
static constexpr uint32_t MAX_BINDINGS_PER_DRAW = 16;
static constexpr uint32_t MAX_FRAMEBUFFER_ATTACHMENTS = 8;

struct uniform_desc {
  uniform_type::type type;
  uint8_t binding;
};

struct shader_uniform_binding_decs {
  std::string_view name;
  uint32_t binding;
};

struct memory {
  uint8_t* data = nullptr;
  size_t size = 0;
};

struct shader_stage {
  enum type {
    VERTEX = 0,
    FRAGMENT,
    COMPUTE,
    GEOMETRY,
    TESS_CONTROL,
    TESS_EVALUATION,

    COUNT
  };
};

struct shader_compile_result {
  struct image {
    std::string name = { };
    int32_t binding = -1;
  };

  struct uniform_member {
    std::string name = { };
    uint32_t offset = 0;
  };

  struct uniform {
    std::string name;
    int32_t binding = -1;
    std::vector<uniform_member> members = { };
  };

  std::string source = { };
  std::vector<image> images = { };
  std::vector<uniform> uniforms = { };
  bool success = false;
};

struct shader_program_desc {
  shader_compile_result vertex;
  shader_compile_result fragment;
};

struct handle_traits {
  static constexpr uint32_t index_mask = 0x000FFFFFu;
  static constexpr uint32_t gen_offset = 20u;
  static constexpr uint32_t gen_mask   = 0xFFFu << gen_offset;

  static inline uint32_t index(uint32_t value) { return value & index_mask; }
  static inline uint32_t gen(uint32_t value) { return value & gen_mask; }
  static inline void increment_gen(uint32_t& value) { value += 1u << gen_offset; }
};

class handle_allocator {
 public:
  uint32_t alloc() {
    if (next_free_ < store_.size()) {
      uint32_t free = store_[next_free_];
      uint32_t id = next_free_ | handle_traits::gen(free);
      store_[next_free_] = id;
      next_free_ = handle_traits::index(free);
      return id;
    }

    uint32_t id = store_.size();
    store_.push_back(id);
    return id;
  }

  void free(uint32_t value) {
    uint32_t index = handle_traits::index(value);
    store_[index] = next_free_ | handle_traits::gen(value);
    handle_traits::increment_gen(store_[index]);
    next_free_ = index;
  }

  void reserve(size_t num) {
    store_.reserve(num);
  }

 private:
  std::vector<uint32_t> store_;
  uint32_t next_free_ = handle_traits::index_mask;
};

template<class ...>
class index_of;

template <typename T, typename... Args>
struct index_of<T, T, Args...> : std::integral_constant<size_t, 0> { };

template <typename T1, typename T2, typename... Args>
struct index_of<T1, T2, Args...> : std::integral_constant<size_t, 1 + index_of<T1, Args...>::value> { };

template<class ...Args>
class handle_allocator_set_base {
 public:
  template<class T>
  [[nodiscard]] T alloc() {
    return T { allocators_[index_of<T, Args...>::value].alloc() };
  }

  template<class T>
  void free(const T& handle) {
    allocators_[index_of<T, Args...>::value].free(handle.id);
  }

 private:
  handle_allocator allocators_[sizeof...(Args)];
};

using handle_allocator_set = handle_allocator_set_base<
    vertexbuf_handle, indexbuf_handle, uniformbuf_handle, framebuf_handle, shader_handle,
    texture_handle, uniform_handle, swap_chain_handle>;