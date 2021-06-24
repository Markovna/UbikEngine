#pragma once

#include "renderer_api.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace gfx::details {

struct TextureFormatInfo {
    GLint internal_format;
    GLenum format;
    GLenum type;
};

class GLContext {
public:
    using WindowHandle = GLFWwindow*;

    explicit GLContext(void* handle) : handle_((WindowHandle) handle) {
      //TODO
//        log::core::Info("New OpenGL context created.");
    }

    void MakeCurrent();
    void SwapBuffers();

    static void Init(GLContext& context);
    static GLContext CreateDefault(const config& config);

private:
    WindowHandle handle_;
};

class GLRendererAPI : public renderer_api {
private:
    struct VertexBuffer {
        uint32_t id{};
        uint32_t size{};
        vertex_layout layout{};
    };

    struct IndexBuffer {
        uint32_t id{};
        uint32_t size{};
    };

    struct FrameBuffer {
        uint32_t id{};
        texture_handle tex_handles[static_config::kFrameBufferMaxAttachments]{};
        uint32_t tex_num{};
        bool destroy_tex{};
    };

    struct Shader {
    public:
        bool TryGetLocation(attribute::binding::type type, uint16_t& location) const;

        uint32_t model_location = 0;
        uint32_t view_location = 0;
        uint32_t proj_location = 0;
        uint16_t enabled_attributes_mask = 0;
        uint16_t attributes_mask = 0;
        uint16_t attribute_locations[attribute::binding::Count] = {};
        uint32_t id = 0;
    };

    struct Texture {
        uint32_t id{};
        bool render_buffer{};
        texture_format::type format{};
        uint32_t width = 0;
        uint32_t height = 0;
    };

    struct UniformInfo {
        char name[64];
    };

public:
    explicit GLRendererAPI(const config& config);
    ~GLRendererAPI() override;

    void CreateVertexBuffer(vertexbuf_handle handle, const void* data, uint32_t data_size, uint32_t size, vertex_layout layout) override;
    void CreateIndexBuffer(indexbuf_handle handle, const void* data, uint32_t data_size, uint32_t size) override;
    void CreateFrameBuffer(framebuf_handle handle, texture_handle*, uint32_t num, bool destroy_tex) override;
    void CreateShader(shader_handle, const std::string& vertex_src, const std::string& fragment_src, const attribute::binding_pack& bindings) override;
    void CreateUniform(uniform_handle, char* name) override;
    void CreateTexture(texture_handle handle, const void* data, uint32_t data_size, uint32_t width, uint32_t height,
                       texture_format::type, texture_wrap wrap, texture_filter filter, texture_flags::mask flags) override;

    void UpdateVertexBuffer(vertexbuf_handle handle, uint32_t offset, const void* data, uint32_t data_size) override;
    void UpdateIndexBuffer(indexbuf_handle handle, uint32_t offset, const void* data, uint32_t data_sizer) override;

    void Destroy(vertexbuf_handle) override;
    void Destroy(indexbuf_handle) override;
    void Destroy(framebuf_handle) override;
    void Destroy(shader_handle) override;
    void Destroy(uniform_handle) override;
    void Destroy(texture_handle) override;

    void RenderFrame(const struct frame&) override;

private:
    GLContext default_context_;
    uint32_t vao_ = 0;
    IndexBuffer index_buffers_[static_config::kIndexBuffersCapacity];
    VertexBuffer vertex_buffers_[static_config::kVertexBuffersCapacity];
    FrameBuffer frame_buffers_[static_config::kVertexBuffersCapacity];
    Shader shaders_[static_config::kShadersCapacity];
    Texture textures_[static_config::kTexturesCapacity];
    UniformInfo uniforms_[static_config::kUniformsCapacity];
};

}