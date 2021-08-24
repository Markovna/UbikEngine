#pragma once

#include "gfx/gfx.h"
#include "core/assets/asset_loader.h"

#include <string>

class shader {
 public:
  shader(const char* vertex_src, const char* fragment_scr, gfx::attribute::binding_pack&);

  shader(const shader&) = delete;
  shader& operator=(const shader&) = delete;

  shader(shader&&) noexcept;
  shader& operator=(shader&&) noexcept;

  ~shader();

  [[nodiscard]] gfx::shader_handle handle() const noexcept { return handle_; }

 private:
  void swap(shader& other);

 private:
  gfx::shader_handle handle_;
};

namespace assets::loader {

template<>
std::unique_ptr<shader> load(std::istream&);

}

