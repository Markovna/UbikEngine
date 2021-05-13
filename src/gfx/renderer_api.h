#ifndef HOT_RELOAD_TEST_SRC_GFX_RENDERER_API_H_
#define HOT_RELOAD_TEST_SRC_GFX_RENDERER_API_H_

#include "gfx.h"

namespace gfx::details {

class renderer_api {
 public:
  renderer_api() = default;
  renderer_api(const renderer_api &) = delete;
  renderer_api(renderer_api &&) = delete;

  renderer_api &operator=(const renderer_api &) = delete;
  renderer_api &operator=(renderer_api &&) = delete;

  virtual ~renderer_api() = default;

  virtual void CreateVertexBuffer(vertexbuf_handle,
                                  const void *data,
                                  uint32_t data_size,
                                  uint32_t size,
                                  vertex_layout layout) = 0;
  virtual void CreateIndexBuffer(indexbuf_handle, const void *data, uint32_t data_size, uint32_t size) = 0;
  virtual void CreateFrameBuffer(framebuf_handle, texture_handle *, uint32_t num, bool destroy_tex) = 0;
  virtual void CreateShader(shader_handle, const std::string &source) = 0;
  virtual void CreateUniform(uniform_handle, char *name) = 0;
  virtual void CreateTexture(texture_handle,
                             const void *data,
                             uint32_t data_size,
                             uint32_t width,
                             uint32_t height,
                             texture_format::type,
                             texture_wrap wrap,
                             texture_filter filter,
                             texture_flags::mask flags) = 0;

  virtual void UpdateVertexBuffer(vertexbuf_handle handle,
                                  uint32_t offset,
                                  const void *data,
                                  uint32_t data_size) = 0;
  virtual void UpdateIndexBuffer(indexbuf_handle handle, uint32_t offset, const void *data, uint32_t data_size) = 0;

  virtual void Destroy(vertexbuf_handle) = 0;
  virtual void Destroy(indexbuf_handle) = 0;
  virtual void Destroy(framebuf_handle) = 0;
  virtual void Destroy(shader_handle) = 0;
  virtual void Destroy(uniform_handle) = 0;
  virtual void Destroy(texture_handle) = 0;

  virtual void RenderFrame(const struct frame&) = 0;
};

}

#endif //HOT_RELOAD_TEST_SRC_GFX_RENDERER_API_H_
