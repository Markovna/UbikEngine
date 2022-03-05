#pragma once

#include "core/ecs.h"
#include "gfx/gfx.h"
#include "renderer.h"
#include "base/delegate.h"
#include "core/meta/interface.h"
#include "base/event.h"
#include "viewer.h"

struct transform_component;

struct view {
  uint32_t sort_key;
  view_projection camera;
};

class render_interface
  : public interface<void(view&,
                          const ecs::details::component_pool<transform_component>*,
                          const ecs::details::component_pool_base*,
                          renderer&,
                          render_command_buffer&,
                          resource_command_buffer&)> {
  using interface::interface;
};

class render_pipeline {
 public:
  template<class It>
  void render(It first, It last, renderer& renderer) {
    auto render_buffer = renderer.create_render_command_buffer();
    auto resource_buffer = renderer.create_resource_command_buffer();

    uint32_t i = 1;
    for (auto it = first; it != last; ++it) {
      uint32_t sort_key = i << 16u;
      render(sort_key, *it, renderer, *render_buffer, *resource_buffer);
      ++i;
    }

    on_render_.invoke(renderer, *render_buffer, *resource_buffer);

    renderer.submit(*resource_buffer);
    renderer.submit(*render_buffer);
  }

  void render(uint32_t sort_key, const viewer& viewer, renderer& renderer, render_command_buffer& render_cmd_buf, resource_command_buffer& resource_cmd_buf);

  template<class ...Args>
  auto on_render_connect(Args&&...args) {
    return on_render_.connect(std::forward<Args>(args)...);
  }

  template<class ...Args>
  void on_render_disconnect(Args&&...args) {
    on_render_.disconnect(std::forward<Args>(args)...);
  }

 private:
  event<renderer&, render_command_buffer&, resource_command_buffer&> on_render_;
};

void init_render_pipeline(const struct systems_registry&);
