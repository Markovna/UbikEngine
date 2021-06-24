#pragma once

#include "gfx/gfx.h"
#include "core/assets/texture.h"
#include "core/assets/shader.h"
#include "core/assets/asset_handle.h"
#include "core/components/component.h"
#include "core/serialization.h"

#include <cstdint>

float* GetVertices();
uint32_t* GetIndices(uint32_t& count);

gfx::vertexbuf_handle GetVertexBuf();
gfx::indexbuf_handle GetIndexBuf();

gfx::uniform_handle GetMainTextureUniform();
gfx::uniform_handle GetSecondTextureUniform();
gfx::uniform_handle GetColorUniform();


//TODO:
class mesh_component : public component<mesh_component> {
 public:
  gfx::uniform_handle GetMainTextureUniform() const {
    return ::GetMainTextureUniform();
  }

  gfx::uniform_handle GetSecondTextureUniform() const {
    return ::GetSecondTextureUniform();
  }

  gfx::uniform_handle GetColorUniform() const {
    return ::GetColorUniform();
  }

  gfx::vertexbuf_handle GetVertexBuf() const {
    return ::GetVertexBuf();
  }

  gfx::indexbuf_handle GetIndexBuf() const {
    return ::GetIndexBuf();
  }

  [[nodiscard]] const texture* GetMainTexture() const {
    return main_texture_.get();
  }

  [[nodiscard]] const texture* GetSecondTexture() const {
    return second_texture_.get();
  }

  [[nodiscard]] const shader* GetShader() const {
    return shader_.get();
  }

  [[nodiscard]] color color() const { return color_; }
  void set_color(const struct color& c) { color_ = c; }

  mesh_component() = default;
  ~mesh_component() = default;

  mesh_component(const mesh_component& o) = default;
  mesh_component(mesh_component&& o) noexcept = default;

  mesh_component& operator=(const mesh_component&) = default;
  mesh_component& operator=(mesh_component&&) noexcept = default;

 private:
  friend serializer<mesh_component>;

 private:
  struct color color_ = color::white();
  texture_handle main_texture_ = assets::load<texture>("assets/textures/container.jpg");
  texture_handle second_texture_ = assets::load<texture>("assets/textures/seal.png");
  shader_handle shader_ = assets::load<shader>("assets/shaders/TestShader.shader");
};

template<>
struct serializer<mesh_component> {
  static void from_asset(const asset&, mesh_component&);
  static void to_asset(asset&, const mesh_component&);
};

