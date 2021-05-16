#pragma once

#include "gfx/gfx.h"
#include "assets.h"

#include <string>

class shader {
 public:
  explicit shader(const std::string& source);

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

namespace assets {

template<>
std::unique_ptr<shader> load_asset(const std::istream&);

}


