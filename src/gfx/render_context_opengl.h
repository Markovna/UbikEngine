#pragma once

#include "render_context.h"

struct swap_chain_gl {
  window::window_handle win_handle;
  vec2i size;
};

struct frame_buffer_gl {
  uint32_t id;
  int32_t swap_chain_index;
  vec2i size;
  texture_handle attachments[MAX_FRAMEBUFFER_ATTACHMENTS];
  uint32_t attachments_size;
};

struct vertex_buffer_gl {
  uint32_t id;
  uint32_t size;
  vertex_layout layout;
};

struct index_buffer_gl {
  uint32_t id;
  uint32_t size;
};

struct uniform_buffer_gl {
  uint32_t id;
  uint32_t size;
};

struct shader_gl {
  uint32_t id;
  int32_t locations[vertex_semantic::COUNT];
};

struct texture_gl {
  uint32_t id;
  bool render_buffer;
  texture_format::type format;
  uint32_t width;
  uint32_t height;
};

struct uniform_binding_gl {
  using uniform_type_flag = flags<uniform_type::type>;
  uniform_type_flag types;
  uint32_t texture_id;
  uint32_t buffer_id;
};

struct uniform_gl {
  using binding_set = std::bitset<MAX_UNIFORM_BINDINGS>;
  uniform_binding_gl bindings[MAX_UNIFORM_BINDINGS];
  binding_set active_bindings;
};

class render_context_opengl : public render_context {
 public:
  static std::unique_ptr<render_context> create();

  std::string_view name() const override;
  void begin_frame() override;
  void submit(const resource_command_buffer&) override;
  void submit(const render_command_buffer&) override;

  void create_swap_chain(window::window_handle, swap_chain_handle, framebuf_handle) override;
  void resize_swap_chain(const swap_chain& sc, vec2i size) override;
  void destroy_swap_chain(swap_chain& swap_chain) override;
  void swap(swap_chain& swap_chain) override;

  ~render_context_opengl() override;

 private:
  void init(window::window_handle);

  void execute(const class create_vertex_buffer_command&);
  void execute(const class create_index_buffer_command&);
  void execute(const class create_uniform_buffer_command&);
  void execute(const class create_texture_command&);
  void execute(const class create_frame_buffer_command&);
  void execute(const class create_uniform_command&);
  void execute(const class create_shader_command&);
  void execute(const class set_uniform_command&);
  void execute(const class update_vertex_buffer_command&);
  void execute(const class update_index_buffer_command&);
  void execute(const class update_uniform_buffer_command&);
  void execute(const class destroy_vertex_buffer_command&);
  void execute(const class destroy_index_buffer_command&);
  void execute(const class destroy_uniform_buffer_command&);
  void execute(const class destroy_texture_command&);
  void execute(const class destroy_frame_buffer_command&);
  void execute(const class destroy_shader_command&);
  void execute(const class destroy_uniform_command&);

 private:
  std::vector<swap_chain_gl> swap_chains_;
  std::vector<frame_buffer_gl> frame_buffers_;
  std::vector<vertex_buffer_gl> vertex_buffers_;
  std::vector<index_buffer_gl> index_buffers_;
  std::vector<uniform_buffer_gl> uniform_buffers_;
  std::vector<shader_gl> shaders_;
  std::vector<texture_gl> textures_;
  std::vector<uniform_gl> uniforms_;

//  int32_t uniform_buffer_offset_alignment_;
  uint32_t vao_;
};
