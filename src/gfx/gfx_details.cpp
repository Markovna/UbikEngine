#include "gfx_details.h"
#include "renderer_api.h"
#include "base/log.h"

namespace gfx::details {

void create_index_buffer_execute(frame_command *frame_command, renderer_api *api) {
  create_index_buffer_command &cmd = std::get<create_index_buffer_command>(*frame_command);
  api->CreateIndexBuffer(cmd.handle, cmd.ptr.get(), cmd.ptr.size(), cmd.size);
  cmd.ptr.reset();
}
void create_vertex_buffer_execute(frame_command *frame_command, renderer_api *api) {
  create_vertex_buffer_command &cmd = std::get<create_vertex_buffer_command>(*frame_command);
  api->CreateVertexBuffer(cmd.handle, cmd.ptr.get(), cmd.ptr.size(), cmd.size, cmd.layout);
  cmd.ptr.reset();
}
void create_frame_buffer_execute(frame_command *frame_command, renderer_api *api) {
  create_frame_buffer_command &cmd = std::get<create_frame_buffer_command>(*frame_command);
  api->CreateFrameBuffer(cmd.handle, cmd.handles, cmd.size, cmd.destroy_tex);
}
void create_uniform_execute(frame_command *frame_command, renderer_api *api) {
  create_uniform_command &cmd = std::get<create_uniform_command>(*frame_command);
  api->CreateUniform(cmd.handle, cmd.name);
}
void update_index_buffer_execute(frame_command *frame_command, renderer_api *api) {
  update_index_buffer_command &cmd = std::get<update_index_buffer_command>(*frame_command);
  api->UpdateIndexBuffer(cmd.handle, cmd.offset, cmd.ptr.get(), cmd.ptr.size());
  cmd.ptr.reset();
}
void update_vertex_buffer_execute(frame_command *frame_command, renderer_api *api) {
  update_vertex_buffer_command &cmd = std::get<update_vertex_buffer_command>(*frame_command);
  api->UpdateVertexBuffer(cmd.handle, cmd.offset, cmd.ptr.get(), cmd.ptr.size());
  cmd.ptr.reset();
}
void create_texture_execute(frame_command *frame_command, renderer_api *api) {
  create_texture_command &cmd = std::get<create_texture_command>(*frame_command);
  api->CreateTexture(cmd.handle,
                     cmd.ptr.get(),
                     cmd.ptr.size(),
                     cmd.width,
                     cmd.height,
                     cmd.format,
                     cmd.wrap,
                     cmd.filter,
                     cmd.flags);
}
void create_shader_execute(frame_command *frame_command, renderer_api *api) {
  create_shader_command &cmd = std::get<create_shader_command>(*frame_command);
  api->CreateShader(cmd.handle, cmd.source);
}
void destroy_index_buffer_execute(frame_command *cmd, renderer_api *api) {
  api->Destroy(std::get<destroy_index_buffer_command>(*cmd).handle);
}
void destroy_vertex_buffer_execute(frame_command *cmd, renderer_api *api) {
  api->Destroy(std::get<destroy_vertex_buffer_command>(*cmd).handle);
}
void destroy_frame_buffer_execute(frame_command *cmd, renderer_api *api) {
  api->Destroy(std::get<destroy_frame_buffer_command>(*cmd).handle);
}
void destroy_texture_execute(frame_command *cmd, renderer_api *api) {
  api->Destroy(std::get<destroy_texture_command>(*cmd).handle);
}
void destroy_uniform_execute(frame_command *cmd, renderer_api *api) {
  api->Destroy(std::get<destroy_uniform_command>(*cmd).handle);
}
void destroy_shader_execute(frame_command *cmd, renderer_api *api) {
  api->Destroy(std::get<destroy_shader_command>(*cmd).handle);
}
template<>
void set_uniform_value<int>(uniform &uniform, int value) {
  uniform.type = details::uniform::Int;
  uniform.int_val = value;
}
template<>
void set_uniform_value<bool>(uniform& uniform, bool value) {
  uniform.type = details::uniform::Bool;
  uniform.bool_val = value;
}

template<>
void set_uniform_value<float>(uniform& uniform, float value) {
  uniform.type = details::uniform::Float;
  uniform.float_val = value;
}
template<>
void set_uniform_value<vec3>(uniform& uniform, vec3 value) {
  uniform.type = details::uniform::Vec3;
  uniform.vec3_val = value;
}
template<>
void set_uniform_value<vec4>(uniform& uniform, vec4 value) {
  uniform.type = details::uniform::Vec4;
  uniform.vec4_val = value;
}
template<>
void set_uniform_value<color>(uniform& uniform, color value) {
  uniform.type = details::uniform::Color;
  uniform.color_val = value;
}
template<>
void set_uniform_value<mat4>(uniform& uniform, mat4 value) {
  uniform.type = details::uniform::Matrix;
  uniform.matrix_val = value;
}

std::vector<execute_func>& frame_commands_execute_table() {
  static std::vector<execute_func> table = {
      create_index_buffer_execute,
      create_vertex_buffer_execute,
      create_frame_buffer_execute,
      create_texture_execute,
      create_uniform_execute,
      create_shader_execute,
      update_vertex_buffer_execute,
      update_index_buffer_execute,
      destroy_index_buffer_execute,
      destroy_vertex_buffer_execute,
      destroy_frame_buffer_execute,
      destroy_texture_execute,
      destroy_uniform_execute,
      destroy_shader_execute
  };
  return table;
}

}

gfx::details::draw_unit &gfx::details::frame::get_draw() {
  assert(draws_count_ < static_config::kMaxDrawCallsCount);
  return draws_[draws_count_];
}

static void clear_draw_unit(gfx::details::draw_unit &draw) {
  draw.transform = mat4::identity();
  draw.ib_handle = {};
  draw.vb_handle = {};
  draw.shader_handle = {};
  draw.camera_id = 0;
  draw.vb_offset = 0;
  draw.vb_size = 0;
  draw.ib_offset = 0;
  draw.ib_size = 0;
  draw.scissor = {};
  draw.options = gfx::options::DepthTest;
  draw.uniforms_size = 0;
  draw.texture_slots_size = 0;
}

void gfx::details::frame::next() {
  clear_draw_unit(draws_[++draws_count_]);
}
void gfx::details::frame::reset() {
  draws_count_ = 0;
  clear_draw_unit(draws_[draws_count_]);
  commands_count_ = 0;
}
void gfx::details::frame::set_cameras(const gfx::details::camera *src) {
  std::memcpy(cameras_, src, sizeof(camera) * static_config::kCamerasCapacity);
}
bool gfx::details::shader_type::try_parse(std::string_view str, gfx::details::shader_type::type &type) {
  if (str == "vertex") {
    type = Vertex;
    return true;
  }
  if (str == "fragment") {
    type = Fragment;
    return true;
  }
  return false;
}
void gfx::details::pre_process_shader(const std::string &source,
                                      std::string &vertex_shader,
                                      std::string &fragment_shader,
                                      gfx::attribute::binding_pack &bindings) {
  {
    static const std::string typeToken = "#type";
    size_t pos = source.find(typeToken, 0);
    shader_type::type type;

    while (pos != std::string::npos) {
      size_t eol = source.find_first_of('\n', pos);
      size_t begin = pos + typeToken.size() + 1;
      std::string typeStr = source.substr(begin, eol - begin);
      uint64_t nextLinePos = eol + 1;
      pos = source.find(typeToken, nextLinePos);

      if (shader_type::try_parse(typeStr, type)) {
        std::string str = (pos == std::string::npos) ?
                          source.substr(nextLinePos) :
                          source.substr(nextLinePos, pos - nextLinePos);
        if (type == shader_type::Vertex) {
          vertex_shader = std::move(str);
        } else if (type == shader_type::Fragment) {
          fragment_shader = std::move(str);
        }
      }
    }
  }

  {
    static const std::string token = "#binding";
    size_t pos = vertex_shader.find(token, 0);
    int loc = 0;
    const char* location_template_str = "layout (location = #)";
    const size_t location_str_pos = 19;
    while (pos != std::string::npos) {
      size_t eol = vertex_shader.find_first_of('\n', pos);
      size_t begin = pos + token.size() + 1;
      std::string binding_type = vertex_shader.substr(begin, eol - begin);

      std::string location_str(location_template_str);
      location_str.replace(location_str_pos, 1, std::to_string(loc));
      vertex_shader.replace(pos, eol - pos, location_str);

      attribute::binding::type binding;
      if (attribute::binding::try_parse(binding_type, binding)) {
        attribute::set_pack(bindings, binding, loc);
      } else {
        logger::core::Error("Can't parse {}", binding_type);
      }

      pos = vertex_shader.find(token, pos);
      loc++;
    }
  }
}

const gfx::details::texture_format_info *gfx::details::get_texture_formats() {
  static const texture_format_info formats[] = {
      { sizeof(uint8_t),    1,   0,   0 }, // R8
      { sizeof(uint8_t),    3,   0,   0 }, // RGB8
      { sizeof(uint8_t),    4,   0,   0 }, // RGBA8
      { 0,    0,   24,  8 }, // D24S8
  };
  return formats;
}
