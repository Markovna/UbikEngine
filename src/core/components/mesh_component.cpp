#include "mesh_component.h"
#include "base/guid.h"

float *GetVertices() {
  static float size = 1.0f;
  static float hs = size * 0.5f;
  static float vertices[] = {
      // pos                // tex coords
      -hs, -hs, -hs,  0.0f, 0.0f,
      hs, -hs, -hs,  1.0f, 0.0f,
      hs,  hs, -hs,  1.0f, 1.0f,
      hs,  hs, -hs,  1.0f, 1.0f,
      -hs,  hs, -hs,  0.0f, 1.0f,
      -hs, -hs, -hs,  0.0f, 0.0f,

      -hs, -hs,  hs,  0.0f, 0.0f,
      hs, -hs,  hs,  1.0f, 0.0f,
      hs,  hs,  hs,  1.0f, 1.0f,
      hs,  hs,  hs,  1.0f, 1.0f,
      -hs,  hs,  hs,  0.0f, 1.0f,
      -hs, -hs,  hs,  0.0f, 0.0f,

      -hs,  hs,  hs,  1.0f, 0.0f,
      -hs,  hs, -hs,  1.0f, 1.0f,
      -hs, -hs, -hs,  0.0f, 1.0f,
      -hs, -hs, -hs,  0.0f, 1.0f,
      -hs, -hs,  hs,  0.0f, 0.0f,
      -hs,  hs,  hs,  1.0f, 0.0f,

      hs,  hs,  hs,  1.0f, 0.0f,
      hs,  hs, -hs,  1.0f, 1.0f,
      hs, -hs, -hs,  0.0f, 1.0f,
      hs, -hs, -hs,  0.0f, 1.0f,
      hs, -hs,  hs,  0.0f, 0.0f,
      hs,  hs,  hs,  1.0f, 0.0f,

      -hs, -hs, -hs,  0.0f, 1.0f,
      hs, -hs, -hs,  1.0f, 1.0f,
      hs, -hs,  hs,  1.0f, 0.0f,
      hs, -hs,  hs,  1.0f, 0.0f,
      -hs, -hs,  hs,  0.0f, 0.0f,
      -hs, -hs, -hs,  0.0f, 1.0f,

      -hs,  hs, -hs,  0.0f, 1.0f,
      hs,  hs, -hs,  1.0f, 1.0f,
      hs,  hs,  hs,  1.0f, 0.0f,
      hs,  hs,  hs,  1.0f, 0.0f,
      -hs,  hs,  hs,  0.0f, 0.0f,
      -hs,  hs, -hs,  0.0f, 1.0f
  };

  return vertices;
}
gfx::vertexbuf_handle GetVertexBuf() {
  uint32_t count;
  GetIndices(count);
  float* vertices = GetVertices();

  static gfx::vertexbuf_handle vb_handle = gfx::create_vertex_buffer(
      gfx::make_ref(vertices, sizeof(float) * 5 * count),
      count,
      {
          {gfx::attribute::binding::Position, gfx::attribute::format::Float3() },
          {gfx::attribute::binding::TexCoord0, gfx::attribute::format::Float2() }
      }
  );
  return vb_handle;
}
gfx::indexbuf_handle GetIndexBuf() {
  uint32_t count;
  uint32_t* indices = GetIndices(count);
  static gfx::indexbuf_handle ib_handle =
      gfx::create_index_buffer(gfx::make_ref(indices, sizeof(uint32_t) * count ), count);
  return ib_handle;
}
uint32_t *GetIndices(uint32_t& count) {
  static uint32_t indices[] = {
      0,  1,  2,
      3,  4,  5,
      6,  7,  8,
      9,  10, 11,
      12, 13, 14,
      15, 16, 17,
      18, 19, 20,
      21, 22, 23,
      24, 25, 26,
      27, 28, 29,
      30, 31, 32,
      33, 34, 35
  };
  count = 36;
  return indices;
}
gfx::uniform_handle GetMainTextureUniform() {
  static gfx::uniform_handle uniform_handle = gfx::create_uniform("Texture1");
  return uniform_handle;
}
gfx::uniform_handle GetSecondTextureUniform() {
  static gfx::uniform_handle uniform_handle = gfx::create_uniform("Texture2");
  return uniform_handle;
}
gfx::uniform_handle GetColorUniform() {
  static gfx::uniform_handle uniform_handle = gfx::create_uniform("mainColor");
  return uniform_handle;
}

void serializer<mesh_component>::from_asset(assets::provider* provider, const asset& asset, mesh_component& comp) {
  assets::get(provider, asset, "color", comp.color_);

  comp.main_texture_ = resources::resolve<texture>(asset, "main_texture", provider);
  comp.second_texture_ = resources::resolve<texture>(asset, "second_texture", provider);
  comp.shader_ = resources::resolve<shader>(asset, "shader", provider);
}

void serializer<mesh_component>::to_asset(asset& asset, const mesh_component& comp) {
  assets::set(asset, "color", comp.color_);
  assets::set(asset, "main_texture", comp.main_texture_.id());
  assets::set(asset, "second_texture", comp.second_texture_.id());
  assets::set(asset, "shader", comp.shader_.id());
}
