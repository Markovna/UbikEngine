#pragma once

#include "core/systems_registry.h"
#include "renderer.h"

class viewport {
 public:
  explicit viewport(const system_ptr<renderer>& renderer)
      : renderer_(renderer)
      , size_()
      , texture_uniform_()
      , framebuf_()
      , color_target_()
      , depth_target_()
  {}

  [[nodiscard]] framebuf_handle target() const { return framebuf_; }
  [[nodiscard]] intptr_t image_id() const { return (intptr_t) texture_uniform_.id; }

  [[nodiscard]] const vec2i& size() const { return size_; }

  void resize(vec2i size);

  void clear();

  ~viewport();

 private:
  void create(resource_command_buffer& cmd_buf);

  void free_resources(resource_command_buffer& cmd_buf);

 private:
  system_ptr<renderer> renderer_;
  vec2i size_;
  uniform_handle texture_uniform_;
  framebuf_handle framebuf_;
  texture_handle color_target_;
  texture_handle depth_target_;
};


