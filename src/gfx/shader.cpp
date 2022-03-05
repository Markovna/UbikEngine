#include "shader.h"

#include <memory>
#include "command_buffers.h"

shader_handle shader::create_shader(const shader_program_desc& desc, resource_command_buffer* res_buf) {
  return res_buf->create_shader(desc);
}