#pragma once

#include "gfx.h"
#include "command_buffers.h"

#include <unordered_map>
#include <string>

class shader {
 public:
  shader(const shader_program_desc& desc, resource_command_buffer* res_buf)
    : handle_(create_shader(desc, res_buf))
  {}

 public:
  inline shader_handle handle() const { return handle_; }

 private:
  static shader_handle create_shader(const shader_program_desc& desc, resource_command_buffer* res_buf) {
    return res_buf->create_shader(desc);
  }

 private:
  shader_handle handle_;
};
