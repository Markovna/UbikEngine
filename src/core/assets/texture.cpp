#include "gfx/gfx_details.h"
#include "texture.h"

#include "stb_image.h"

namespace assets {

class texture_loader {
 public:
  texture_loader(const texture_loader& loader) = delete;
  texture_loader& operator=(const texture_loader& loader) = delete;

  texture_loader(texture_loader&& loader) noexcept = delete;
  texture_loader& operator=(texture_loader&& loader) noexcept = delete;

  explicit texture_loader(const std::istream &in) {
    stbi_set_flip_vertically_on_load(true);
    std::stringstream ss;
    ss << in.rdbuf();
    std::string str = ss.str();
    data_ = stbi_load_from_memory(reinterpret_cast<const stbi_uc *>(str.c_str()), str.length(), &width_, &height_, &channels_, 0);
  }

  ~texture_loader() {
    if (data_) {
      stbi_image_free(data_);
      data_ = nullptr;
    }
  }

  explicit operator bool() const { return data_; }

  [[nodiscard]] int height() const noexcept { return height_; }
  [[nodiscard]] int width() const noexcept { return width_; }
  [[nodiscard]] int channels() const noexcept { return channels_; }
  [[nodiscard]] const uint8_t* data() const noexcept { return data_; }

 private:
  uint8_t* data_;
  int width_, height_, channels_;
};

static gfx::texture_format::type to_format(uint32_t channels) {
  if (channels == 1) return gfx::texture_format::R8;
  if (channels == 3) return gfx::texture_format::RGB8;
  return gfx::texture_format::RGBA8;
}

template<>
std::unique_ptr<texture> load_asset(const std::istream& stream) {
  texture_loader loader(stream);
  if (loader) {
    const uint8_t* data = loader.data();
    uint32_t width = loader.width();
    uint32_t height = loader.height();
    auto format = to_format(loader.channels());
    return std::unique_ptr<texture>(new texture(
        gfx::copy(data, sizeof(uint8_t) * width * height * gfx::details::get_texture_formats()[format].channels),
        width, height,
        format,
        gfx::texture_wrap{}, gfx::texture_filter{},
        gfx::texture_flags::None
    ));
  }

  logger::core::Error("Failed to load texture");
  return nullptr;
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
