#include "render_context_opengl.h"
#include "command_buffers.h"
#include "memory.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define GL_API_PROFILE 1
#define GL_API_DEBUG 1

#if GL_API_PROFILE
#   define UBIK_GL_PROFILE_SCOPE(__name) UBIK_PROFILE_SCOPE(__name)
#   define UBIK_GL_PROFILE_FUNCTION() UBIK_PROFILE_FUNCTION()
#else
#   define UBIK_GL_PROFILE_SCOPE(__name)
#   define UBIK_GL_PROFILE_FUNCTION()
#endif

#if GL_API_DEBUG
#	define GL_ERRORS(__func) _CHECK_GL_ERRORS(__func)
#else
#   define GL_ERRORS(__func) __func
#endif // GL_API_DEBUG

#define _CHECK_GL_ERRORS(__func) \
    __func, check_errors(logger::format("\n{0}:{1}", __FILE__, __LINE__))

static void check_errors(const std::string_view msg) {
  GLenum err = glGetError();
  if (err != GL_NO_ERROR) {
    logger::core::Error("GLRendererAPI error #{0}: {1}", err, msg);
  }
}

namespace experimental::gfx {

std::unique_ptr<render_context> render_context_opengl::create() {
  return std::make_unique<render_context_opengl>();
}

template<class T, class Index, template<class...> class Container>
static T& assure(Container<T>& container, Index index) {
  if (index + 1 > container.size())
    container.resize(index + 1);

  return container[index];
}

static void check_link_status(uint32_t id) {
  int success;
  glGetProgramiv(id, GL_LINK_STATUS, &success);
  if (!success) {
    char log[512];
    glGetProgramInfoLog(id, 512, nullptr, log);
    logger::core::Error("GL::Shader linking failed with errors:\n{}", log);
  }
}

static bool check_compile_status(uint32_t id) {
  int success;
  glGetShaderiv(id, GL_COMPILE_STATUS, &success);
  if (!success) {
    char log[512];
    glGetShaderInfoLog(id, 512, nullptr, log);
    logger::core::Error("GL::Shader compilation failed with errors:\n{}", log);
  }
  return success;
}

static void bind_uniform(uniform_gl& uniform) {
  for (uint32_t i = 0; i < uniform.bindings_count; i++) {
    uniform_binding_gl& binding = uniform.bindings[i];

    if (binding.type == uniform_type::BUFFER) {
      GL_ERRORS(glBindBufferRange(GL_UNIFORM_BUFFER, binding.binding, binding.buffer_id, binding.offset, binding.size));
    } else if (binding.type == uniform_type::SAMPLER) {
      // TODO: glBindTextureUnit
      GL_ERRORS(glActiveTexture(GL_TEXTURE0 + binding.binding));
      GL_ERRORS(glBindTexture(GL_TEXTURE_2D, binding.texture_id));
    }
  }
}

constexpr static GLenum vertex_data_types[] = {
    GL_FLOAT,
    GL_BYTE,
    GL_SHORT,
    GL_INT,
    GL_UNSIGNED_BYTE,
    GL_UNSIGNED_SHORT,
    GL_UNSIGNED_INT,
};

static_assert(sizeof(vertex_data_types) == vertex_type::COUNT * sizeof(GLenum),
              "Missing some vertex type in types array.");

struct texture_format_info_gl {
  GLint internal_format;
  GLenum format;
  GLenum type;
};

constexpr static texture_format_info_gl texture_formats[] = {
    { GL_RED,                   GL_RED,                     GL_UNSIGNED_BYTE },     // R8
    { GL_RGB,                   GL_RGB,                     GL_UNSIGNED_BYTE },     // RGB8
    { GL_RGBA,                  GL_RGBA,                    GL_UNSIGNED_BYTE },     // RGBA8
    { GL_DEPTH24_STENCIL8,      GL_DEPTH_STENCIL,           GL_UNSIGNED_INT_24_8 }, // D24S8
};

static_assert(sizeof(texture_formats) == texture_format::COUNT * sizeof(texture_format_info_gl),
              "Missing some texture format type in texture_formats array.");

void render_context_opengl::create_swap_chain(
    window::window_handle win_handle,
    swap_chain_handle sc_handle,
    framebuf_handle fb_handle) {

  if (swap_chains_.empty()) {
    init(win_handle);
  }

  uint32_t sc_index = handle_traits::index(sc_handle.id);
  uint32_t fb_index = handle_traits::index(fb_handle.id);

  swap_chain_gl& sc = assure(swap_chains_, sc_index);
  frame_buffer_gl& fb = assure(frame_buffers_, fb_index);

  vec2i sc_size { };
  glfwGetFramebufferSize((GLFWwindow*) win_handle, &sc_size.x, &sc_size.y);
  sc.win_handle = win_handle;
  sc.size = sc_size;

  fb.id = 0;
  fb.swap_chain_index = sc_index;
}

void render_context_opengl::destroy_swap_chain(swap_chain &swap_chain) {

}

void render_context_opengl::swap(swap_chain& sc) {
  uint32_t sc_index = handle_traits::index(sc.handle.id);
  glfwSwapBuffers((GLFWwindow*) swap_chains_[sc_index].win_handle);
}

void render_context_opengl::submit(const resource_command_buffer* command_buffer) {
  for (auto& header : *command_buffer) {
    switch (header.type) {
      case resource_command_type::CREATE_VERTEX_BUFFER: {
        execute(get_command<create_vertex_buffer_command>(header));
        break;
      }
      case resource_command_type::CREATE_INDEX_BUFFER: {
        execute(get_command<create_index_buffer_command>(header));
        break;
      }
      case resource_command_type::CREATE_TEXTURE: {
        execute(get_command<create_texture_command>(header));
        break;
      }
      case resource_command_type::CREATE_UNIFORM: {
        execute(get_command<create_uniform_command>(header));
        break;
      }
      case resource_command_type::CREATE_SHADER: {
        execute(get_command<create_shader_command>(header));
        break;
      }
      case resource_command_type::UPDATE_UNIFORM: {
        execute(get_command<update_uniform_command>(header));
        break;
      }
      case resource_command_type::UPDATE_VERTEX_BUFFER: {
        execute(get_command<update_vertex_buffer_command>(header));
        break;
      }
      case resource_command_type::UPDATE_INDEX_BUFFER: {
        execute(get_command<update_index_buffer_command>(header));
        break;
      }
      case resource_command_type::DESTROY_VERTEX_BUFFER: {
        execute(get_command<destroy_vertex_buffer_command>(header));
        break;
      }
      case resource_command_type::DESTROY_INDEX_BUFFER: {
        execute(get_command<destroy_index_buffer_command>(header));
        break;
      }
      case resource_command_type::DESTROY_TEXTURE: {
        execute(get_command<destroy_texture_command>(header));
        break;
      }
      case resource_command_type::DESTROY_UNIFORM: {
        execute(get_command<destroy_uniform_command>(header));
        break;
      }
      case resource_command_type::DESTROY_SHADER: {
        execute(get_command<destroy_shader_command>(header));
        break;
      }
      case resource_command_type::CREATE_FRAME_BUFFER: {
        execute(get_command<create_frame_buffer_command>(header));
        break;
      }
      case resource_command_type::DESTROY_FRAME_BUFFER: {
        execute(get_command<destroy_frame_buffer_command>(header));
        break;
      }
    }
  }
}

void render_context_opengl::execute(const create_frame_buffer_command& cmd) {
  frame_buffer_gl& fb = frame_buffers_[handle_traits::index(cmd.handle.id)];
  fb.attachments_size = cmd.attachments_size;
  fb.swap_chain_index = 0;

  glGenFramebuffers(1, &fb.id);
  glBindFramebuffer(GL_FRAMEBUFFER, fb.id);

  uint32_t color_attachment_i = 0;
  for (uint32_t i = 0; i < cmd.attachments_size; i++) {
    texture_handle handle = fb.attachments[i] = cmd.attachments[i];
    texture_gl& attachment = textures_[handle_traits::index(handle.id)];

    GLenum attachment_type;
    const texture_format_info& format = texture_format::info[attachment.format];
    bool is_depth = format.depth_bits || format.stencil_bits;
    if (is_depth) {
      if (format.stencil_bits && format.depth_bits) {
        attachment_type = GL_DEPTH_STENCIL_ATTACHMENT;
      } else if (format.depth_bits) {
        attachment_type = GL_DEPTH_ATTACHMENT;
      } else {
        attachment_type = GL_STENCIL_ATTACHMENT;
      }
    } else {
      attachment_type = GL_COLOR_ATTACHMENT0 + color_attachment_i;
      color_attachment_i++;
    }

    if (attachment.render_buffer) {
      GL_ERRORS(glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachment_type, GL_RENDERBUFFER, attachment.id));
    } else {
      GL_ERRORS(glFramebufferTexture2D(GL_FRAMEBUFFER, attachment_type, GL_TEXTURE_2D, attachment.id, 0));
    }
  }

  assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void render_context_opengl::execute(const destroy_frame_buffer_command& cmd) {
  frame_buffer_gl& fb = frame_buffers_[handle_traits::index(cmd.handle.id)];

  glDeleteFramebuffers(1, &fb.id);
}

void render_context_opengl::execute(const update_vertex_buffer_command& cmd) {
  vertex_buffer_gl& vb = vertex_buffers_[handle_traits::index(cmd.handle.id)];

  glBindBuffer(GL_ARRAY_BUFFER, vb.id);
  GL_ERRORS(glBufferSubData(GL_ARRAY_BUFFER, cmd.offset, cmd.memory.size, cmd.memory.data));
  glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void render_context_opengl::execute(const update_index_buffer_command& cmd) {
  index_buffer_gl& ib = index_buffers_[handle_traits::index(cmd.handle.id)];

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ib.id);
  GL_ERRORS(glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, cmd.offset, cmd.memory.size, cmd.memory.data));
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void render_context_opengl::execute(const update_uniform_command& cmd) {
  uniform_gl& uniform = uniforms_[handle_traits::index(cmd.handle.id)];

  uint32_t index = uniform.bindings_index[cmd.binding];
  uniform_binding_gl& binding = uniform.bindings[index];

  if (binding.type == uniform_type::SAMPLER) {

    binding.texture_id = textures_[handle_traits::index(cmd.texture.id)].id;

  } else if (binding.type == uniform_type::BUFFER) {

    const uint32_t size = cmd.size;
    const uint32_t alignment = uniform_buffer_offset_alignment_;

    uniform_buffer buffer = uniform_buffer_pool_.get_buffer(size, alignment);
    uint32_t aligned_offset = buffer.offset % alignment ? alignment - (buffer.offset % alignment) : 0;
    binding.offset = buffer.offset + aligned_offset;
    binding.size = size;
    binding.buffer_id = buffer.id;

    GL_ERRORS(glBindBuffer(GL_UNIFORM_BUFFER, binding.buffer_id));
    GL_ERRORS(glBufferSubData(GL_UNIFORM_BUFFER, binding.offset, binding.size, cmd.buffer));
    GL_ERRORS(glBindBuffer(GL_UNIFORM_BUFFER, 0));

    buffer.offset += aligned_offset + size;
  }
}

static bool compile_and_attach_shader(uint32_t shader_type, char* source, uint32_t program_id) {
  uint32_t id = glCreateShader(shader_type);
  GL_ERRORS(glShaderSource(id, 1, &source, nullptr));
  GL_ERRORS(glCompileShader(id));
  bool success = check_compile_status(id);

  if (success) {
    GL_ERRORS(glAttachShader(program_id, id));
  }

  GL_ERRORS(glDeleteShader(id));
  return success;
}

void render_context_opengl::execute(const create_shader_command& cmd) {
  shader_gl& shader = assure(shaders_, handle_traits::index(cmd.handle.id));
  shader.id = glCreateProgram();

  compile_and_attach_shader(GL_VERTEX_SHADER, reinterpret_cast<char*>(cmd.vertex.data), shader.id);
  compile_and_attach_shader(GL_FRAGMENT_SHADER, reinterpret_cast<char*>(cmd.fragment.data), shader.id);

  GL_ERRORS(glLinkProgram(shader.id));
  check_link_status(shader.id);

  for (uint32_t i = 0; i < vertex_semantic::COUNT; i++) {
    shader.locations[i] = glGetAttribLocation(shader.id, vertex_semantic::names[i]);
  }

  // TODO: check & cache on init
  bool supported_420 = glfwExtensionSupported("GL_ARB_shading_language_420pack");
  if (!supported_420) {
    glUseProgram(shader.id);

    // TODO: parse bindings
//    void* ptr = cmd.desc.metadata.data;
//    uint32_t pos = 0;
//    while (pos < cmd.desc.metadata.size) {
//      char* name = reinterpret_cast<char*>(ptr);
//      uint32_t name_size = std::strlen(name);
//
//      uint8_t* next = reinterpret_cast<uint8_t*>(ptr) + name_size;
//      uint8_t type = *(++next);
//      uint8_t binding = *(++next);
//
//      if (type == uniform_type::BUFFER) {
//        int32_t ubo_index = glGetUniformBlockIndex(shader.id, name);
//        if (ubo_index != GL_INVALID_INDEX) {
//          glUniformBlockBinding(shader.id, ubo_index, binding);
//        }
//      } else if (type == uniform_type::SAMPLER) {
//        int32_t location = glGetUniformLocation(shader.id, name);
//        if (location >= 0) {
//          glUniform1i(location, binding);
//        }
//      }
//
//      ptr = ++next;
//      pos += name_size + 3;
//    }

    glUseProgram(0);
  }
}

void render_context_opengl::execute(const destroy_shader_command& cmd) {
  GL_ERRORS(glDeleteProgram(shaders_[handle_traits::index(cmd.handle.id)].id));
}

void render_context_opengl::execute(const destroy_uniform_command& cmd) {

}

void render_context_opengl::execute(const create_uniform_command& cmd) {
  uniform_gl& uniform = assure(uniforms_, handle_traits::index(cmd.handle.id));
  for (uint32_t i = 0; i < cmd.uniforms_size; ++i) {
    uniform.bindings[i].binding = cmd.uniforms[i].binding;
    uniform.bindings[i].type = cmd.uniforms[i].type;
    uniform.bindings_index[uniform.bindings[i].binding] = i;
  }
  uniform.bindings_count = cmd.uniforms_size;
}

void render_context_opengl::execute(const create_vertex_buffer_command& cmd) {
  vertex_buffer_gl& vb = assure(vertex_buffers_, handle_traits::index(cmd.handle.id));

  GL_ERRORS(glGenBuffers(1, &vb.id));
  glBindBuffer(GL_ARRAY_BUFFER, vb.id);
  // TODO: (gfx) GL_STATIC_DRAW / GL_DYNAMIC_DRAW
  GL_ERRORS(glBufferData(GL_ARRAY_BUFFER, cmd.memory.size, cmd.memory.data, GL_DYNAMIC_DRAW));
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  vb.size = cmd.memory.size;
  vb.layout = cmd.layout;
}

void render_context_opengl::submit(const render_command_buffer* command_buffer) {

  glBindVertexArray(vao_);

  GL_ERRORS(glClearColor(0.2f, 0.2f, 0.2f, 1.0f));
  GL_ERRORS(glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT));

  swap_chain_gl* sc = nullptr;

  for (auto& header : *command_buffer) {
    switch (header.type) {
      case render_command_type::BIND_RENDER_PASS: {
        bind_render_pass_command& cmd = get_command<bind_render_pass_command>(header);
        frame_buffer_gl& fb = frame_buffers_[handle_traits::index(cmd.handle.id)];
        sc = &swap_chains_[fb.swap_chain_index];

        glfwMakeContextCurrent((GLFWwindow*) sc->win_handle);
        GL_ERRORS(glBindFramebuffer(GL_FRAMEBUFFER, fb.id));
        GL_ERRORS(glViewport(0, 0, sc->size.x, sc->size.y));
        break;
      }
      case render_command_type::SET_VIEWPORT: {
        assert(sc);

        set_viewport_command& cmd = get_command<set_viewport_command>(header);
        GL_ERRORS(glViewport(
            cmd.viewport.x,
            sc->size.y - (cmd.viewport.y + cmd.viewport.w),
            cmd.viewport.z,
            cmd.viewport.w
        ));
        break;
      }
      case render_command_type::DRAW: {
        assert(sc);

        draw_command& cmd = get_command<draw_command>(header);
        shader_gl& shader = shaders_[handle_traits::index(cmd.shader.id)];
        vertex_buffer_gl& vb = vertex_buffers_[handle_traits::index(cmd.vb_handle.id)];

        GL_ERRORS(glUseProgram(shader.id));

        for (uint32_t i = 0; i < cmd.bindings.uniforms_count; i++) {
          uniform_gl& uniform = uniforms_[handle_traits::index(cmd.bindings.uniforms[i].id)];
          bind_uniform(uniform);
        }

        GL_ERRORS(glBindBuffer(GL_ARRAY_BUFFER, vb.id));

        uint32_t base_vertex = cmd.vertex_offset * vb.layout.stride;
        for (uint32_t i = 0; i < vertex_semantic::COUNT; i++) {
          vertex_semantic::type semantic = (vertex_semantic::type) i;
          vertex_layout::item item = vb.layout.items[semantic];
          int32_t location = shader.locations[semantic];
          if (location >= 0 && item.size) {
            GL_ERRORS(glVertexAttribPointer(
              location,
              item.size,
              vertex_data_types[item.type],
              item.normalized,
              vb.layout.stride,
              (void*) (uintptr_t) (base_vertex + item.offset)
            ));
            GL_ERRORS(glEnableVertexAttribArray(location));
          } else if (location >= 0) {
            GL_ERRORS(glDisableVertexAttribArray(location));
          }
        }

        if (cmd.ib_handle) {
          index_buffer_gl& ib = index_buffers_[handle_traits::index(cmd.ib_handle.id)];

          GL_ERRORS(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ib.id));
          GL_ERRORS(glDrawElements(
            GL_TRIANGLES,
            cmd.size ? cmd.size : ib.size,
            GL_UNSIGNED_INT,
            (void *) (size_t) (cmd.index_offset * sizeof(uint32_t))
          ));
          glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
          glBindBuffer(GL_ARRAY_BUFFER, 0);

        } else {

          GL_ERRORS(glDrawArrays(
            GL_TRIANGLES,
            0,
            cmd.size ? cmd.size : vb.size
          ));
          glBindBuffer(GL_ARRAY_BUFFER, 0);
        }

        glUseProgram(0);

        break;
      }
    }
  }

  glBindVertexArray(0);
}

void render_context_opengl::begin_frame() {
  uniform_buffer_pool_.reset();
}

render_context_opengl::~render_context_opengl() {
  glBindVertexArray(0);
  glDeleteVertexArrays(1, &vao_);
}

void render_context_opengl::init(window::window_handle win_handle) {

  glfwMakeContextCurrent((GLFWwindow*) win_handle);

  int loaded = gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
  assert(loaded);

  glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &uniform_buffer_offset_alignment_);

  uniform_buffer_pool_.allocate();

  glGenVertexArrays(1, &vao_);

  GL_ERRORS(glEnable(GL_DEPTH_TEST));
  GL_ERRORS(glEnable(GL_BLEND));
  GL_ERRORS(glBlendEquation(GL_FUNC_ADD));
  GL_ERRORS(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
  GL_ERRORS(glDisable(GL_CULL_FACE));
}
std::string_view render_context_opengl::name() const {
  constexpr static const char* name = "OpenGL";
  return { name };
}

void render_context_opengl::execute(const create_index_buffer_command& cmd) {
  index_buffer_gl& ib = assure(index_buffers_, handle_traits::index(cmd.handle.id));

  GL_ERRORS(glGenBuffers(1, &ib.id));
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ib.id);
  GL_ERRORS(glBufferData(GL_ELEMENT_ARRAY_BUFFER, cmd.memory.size, cmd.memory.data, GL_STATIC_DRAW));
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

  ib.size = cmd.memory.size;
}

void render_context_opengl::execute(const create_texture_command& cmd) {
  texture_gl& texture = assure(textures_, handle_traits::index(cmd.handle.id));
  texture.render_buffer = cmd.desc.flags[texture_flag::RENDER_TARGET];
  texture.format = cmd.desc.format;
  texture.width = cmd.desc.width;
  texture.height = cmd.desc.height;

  const texture_format_info_gl& format = texture_formats[cmd.desc.format];
  if (texture.render_buffer) {

    GL_ERRORS(glGenRenderbuffers(1, &texture.id));
    GL_ERRORS(glBindRenderbuffer(GL_RENDERBUFFER, texture.id));
    GL_ERRORS(glRenderbufferStorage(GL_RENDERBUFFER, format.internal_format, cmd.desc.width, cmd.desc.height));
    GL_ERRORS(glBindRenderbuffer(GL_RENDERBUFFER, 0));

  } else {

    static GLenum wraps[] = {
        GL_REPEAT, GL_MIRRORED_REPEAT, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_BORDER
    };

    static GLenum mag_filters[] = {
        GL_LINEAR, GL_NEAREST
    };

    static GLenum min_filters[][3] = {
        { GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, GL_NEAREST_MIPMAP_LINEAR},
        { GL_NEAREST, GL_LINEAR_MIPMAP_NEAREST, GL_NEAREST_MIPMAP_NEAREST}
    };

    const bool has_mipmaps = true;

    GL_ERRORS(glGenTextures(1, &texture.id));

    GL_ERRORS(glBindTexture(GL_TEXTURE_2D, texture.id));

    GL_ERRORS(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wraps[cmd.desc.wrap.u]));
    GL_ERRORS(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wraps[cmd.desc.wrap.v]));
    GL_ERRORS(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, wraps[cmd.desc.wrap.w]));

    GL_ERRORS(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filters[cmd.desc.filter.min][has_mipmaps ? cmd.desc.filter.map : 0]));
    GL_ERRORS(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filters[cmd.desc.filter.mag]));

    GL_ERRORS(glTexImage2D(GL_TEXTURE_2D, 0, format.internal_format, cmd.desc.width, cmd.desc.height, 0, format.format, format.type, cmd.memory.data));

    if (has_mipmaps) {
      GL_ERRORS(glGenerateMipmap(GL_TEXTURE_2D));
    }

    GL_ERRORS(glBindTexture(GL_TEXTURE_2D, 0));
  }
}

void render_context_opengl::execute(const destroy_texture_command& cmd) {
  texture_gl& texture = textures_[handle_traits::index(cmd.handle.id)];
  if (texture.render_buffer) {
    GL_ERRORS(glDeleteRenderbuffers(1, &texture.id));
  } else {
    GL_ERRORS(glDeleteTextures(1, &texture.id));
  }
}

void render_context_opengl::execute(const class destroy_vertex_buffer_command& cmd) {
  GL_ERRORS(glDeleteBuffers(1, &vertex_buffers_[handle_traits::index(cmd.handle.id)].id));
}

void render_context_opengl::execute(const class destroy_index_buffer_command& cmd) {
  GL_ERRORS(glDeleteBuffers(1, &index_buffers_[handle_traits::index(cmd.handle.id)].id));
}

void uniform_buffer_pool::allocate() {
  uniform_buffer& buffer = buffers_.emplace_back();
  buffer.size = 3 << 20;
  buffer.offset = 0;

  GL_ERRORS(glGenBuffers(1, &buffer.id));

  GL_ERRORS(glBindBuffer(GL_UNIFORM_BUFFER, buffer.id));
  GL_ERRORS(glBufferData(GL_UNIFORM_BUFFER, buffer.size, nullptr, GL_DYNAMIC_DRAW));
  GL_ERRORS(glBindBuffer(GL_UNIFORM_BUFFER, 0));
}

void uniform_buffer_pool::reset() {
  for (uniform_buffer& buffer : buffers_) {
    buffer.offset = 0;
  }
}

uniform_buffer uniform_buffer_pool::get_buffer(uint32_t size, uint32_t alignment) {
  return buffers_.back();
}

}