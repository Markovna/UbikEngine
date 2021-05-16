#pragma once

#include "gfx/gfx.h"
#include "assets.h"

class texture {
 public:
  texture(
      void* data,
      uint32_t width,
      uint32_t height,
      gfx::texture_format::type,
      gfx::texture_wrap,
      gfx::texture_filter,
      gfx::texture_flags::mask
      );
  
  texture(
      gfx::buffer_ptr,
      uint32_t width,
      uint32_t height,
      gfx::texture_format::type,
      gfx::texture_wrap,
      gfx::texture_filter,
      gfx::texture_flags::mask
  );

  texture(const texture&) = delete;
  texture& operator=(const texture&) = delete;

  texture(texture&&) noexcept;
  texture& operator=(texture&&) noexcept;

  ~texture();

  [[nodiscard]] uint32_t width() const noexcept { return width_; }
  [[nodiscard]] uint32_t height() const noexcept { return height_; }
  [[nodiscard]] gfx::texture_handle handle() const noexcept { return handle_; }

 private:
  void swap(texture& other) noexcept;

 private:
  gfx::texture_handle handle_;
  uint32_t width_, height_;
};

namespace assets {

template<>
std::unique_ptr<texture> load_asset(const std::istream&);

}


