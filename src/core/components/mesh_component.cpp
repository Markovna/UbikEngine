#include "mesh_component.h"

float *GetVertices() {
  static float vertices[] = {
      // pos                // tex coords
      -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
      0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
      0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
      0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
      -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
      -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

      -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
      0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
      0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
      0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
      -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
      -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

      -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
      -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
      -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
      -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
      -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
      -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

      0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
      0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
      0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
      0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
      0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
      0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

      -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
      0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
      0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
      0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
      -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
      -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

      -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
      0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
      0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
      0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
      -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
      -0.5f,  0.5f, -0.5f,  0.0f, 1.0f
  };

  return vertices;
}
gfx::vertexbuf_handle GetVertexBuf() {
  uint32_t count;
  uint32_t* indices = GetIndices(count);
  float* vertices = GetVertices();

  static gfx::vertexbuf_handle vb_handle = gfx::create_vertex_buffer(
      gfx::make_ref(vertices, sizeof(float) * 5 * count),
      count,
      {
          {gfx::attribute::binding::Position,  gfx::attribute::format::Vec3() },
          {gfx::attribute::binding::TexCoord0, gfx::attribute::format::Vec2() }
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

void serialization<mesh_component>::from_asset(const asset& asset, mesh_component& comp) {
  assets::get(asset, "color", comp.color_);
}

void serialization<mesh_component>::to_asset(asset& asset, const mesh_component& comp) {
  assets::set(asset, "color", comp.color_);
}
