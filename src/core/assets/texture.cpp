#include "gfx/gfx_details.h"
#include "texture.h"

#include <memory>
#include "base/log.h"

#include "stb_image.h"

namespace assets {

static gfx::texture_format::type to_format(uint32_t channels) {
  if (channels == 1) return gfx::texture_format::R8;
  if (channels == 3) return gfx::texture_format::RGB8;
  return gfx::texture_format::RGBA8;
}

template<>
std::unique_ptr<texture> assets::loader::load(std::istream& stream) {

  uint32_t width, height, channels;

  stream.read((char*) &width, sizeof(width));
  stream.read((char*) &height, sizeof(height));
  stream.read((char*) &channels, sizeof(channels));

  uint32_t size = width * height * channels;
  uint8_t buffer[size];
  stream.read((char*) buffer, size);

  return std::make_unique<texture>(
      gfx::copy(buffer, size),
      width, height,
      to_format(channels),
      gfx::texture_wrap{}, gfx::texture_filter{},
      gfx::texture_flags::None
  );
}

}

texture::texture(
    void *data,
    uint32_t width,
    uint32_t height,
    gfx::texture_format::type format,
    gfx::texture_wrap wrap,
    gfx::texture_filter filter,
    gfx::texture_flags::mask flags)
  : texture(
      data ? gfx::copy(data, width * height * gfx::details::get_texture_formats()[format].channel_size * gfx::details::get_texture_formats()[format].channels)
           : gfx::buffer_ptr(),
      width,
      height,
      format,
      wrap,
      filter,
      flags
      )
{
  gfx::details::get_texture_formats();
}

texture::texture(texture&& other) noexcept
  : handle_()
  , width_(0)
  , height_(0)
{
  swap(other);
}

texture &texture::operator=(texture&& other) noexcept {
  texture(std::move(other)).swap(*this);
  return *this;
}

void texture::swap(texture &other) noexcept {
  std::swap(handle_, other.handle_);
  std::swap(width_, other.width_);
  std::swap(height_, other.height_);
}

texture::~texture() {
  if (handle_)
    gfx::destroy(handle_);
}

texture::texture(
    gfx::buffer_ptr ptr,
    uint32_t width,
    uint32_t height,
    gfx::texture_format::type format,
    gfx::texture_wrap wrap,
    gfx::texture_filter filter,
    gfx::texture_flags::mask flags)
  : handle_(gfx::create_texture(width, height, format, wrap, filter, flags, std::move(ptr)))
  , width_(width)
  , height_(height)
{}
