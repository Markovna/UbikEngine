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
//  logger::core::Info("Start loading texture");

  if (!stream) {
    logger::core::Error("Couldn't load texture");
    return {};
  }

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

namespace assets::compiler {

template<>
bool compile<texture>(std::ifstream& stream, const asset& meta, std::ostream& output) {
  stbi_set_flip_vertically_on_load(true);

  std::size_t size = stream.rdbuf()->pubseekoff(0, std::ios::end, std::ios_base::in);
  char buffer [size];
  stream.rdbuf()->pubseekpos(0, std::ios_base::in);
  stream.rdbuf()->sgetn(buffer, size);

  int32_t width, height, channels, desired_channels = 0;
  uint8_t* data = stbi_load_from_memory(reinterpret_cast<stbi_uc*>(buffer), size, &width, &height, &channels, desired_channels);

  output.write((char*) &width, sizeof(width));
  output.write((char*) &height, sizeof(height));
  output.write((char*) &channels, sizeof(channels));
  output.write((char*) data, width * height * channels);
  stbi_image_free(data);

  return true;
}

}
