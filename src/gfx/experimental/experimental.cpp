#include <platform/window.h>
#include <gfx/experimental/gfx.h>
#include <gfx/experimental/renderer.h>
#include <gfx/experimental/render_context_opengl.h>
#include <gfx/experimental/vertex_layout_desc.h>
#include <gfx/experimental/opengl_shader_desc.h>
#include "GLFW/glfw3.h"
#include "stb_image.h"

#include <sstream>

using namespace experimental;

gfx::texture_handle load_texture(std::ifstream& stream, gfx::resource_command_buffer* command_buf) {
  if (!stream) {
    std::cout << "Couldn't load texture";
    return {};
  }

  stbi_set_flip_vertically_on_load(true);

  std::size_t size = stream.rdbuf()->pubseekoff(0, std::ios::end, std::ios_base::in);
  char buffer[size];
  stream.rdbuf()->pubseekpos(0, std::ios_base::in);
  stream.rdbuf()->sgetn(buffer, size);

  int32_t width, height, channels, desired_channels = 0;
  uint8_t* data = stbi_load_from_memory(reinterpret_cast<stbi_uc*>(buffer), size, &width, &height, &channels, desired_channels);

  gfx::memory mem;
  gfx::texture_handle texture = command_buf->create_texture(
      gfx::texture_desc {
          .width = (uint32_t) width,
          .height = (uint32_t) height,
          .format = channels == 3 ? gfx::texture_format::RGB8 : gfx::texture_format::RGBA8,
      },
      mem
    );

  std::memcpy(mem.data, data, mem.size);
  stbi_image_free(data);
  return texture;
}


int main(int argc, char* argv[]) {

  logger::init(std::filesystem::current_path().append("log").c_str());

  window window({512, 512});

  gfx::renderer renderer { gfx::render_context_opengl::create };

  gfx::resource_command_buffer* resource_command_buffer = renderer.create_resource_command_buffer();

  std::ifstream file0 { std::filesystem::current_path().append("assets/textures/container.jpg"), std::ios::in };
  gfx::texture_handle texture0 = load_texture(file0, resource_command_buffer);

  std::ifstream file1 { std::filesystem::current_path().append("assets/textures/seal.png"), std::ios::in };
  gfx::texture_handle texture1 = load_texture(file1, resource_command_buffer);

  char vertex_shader[] = "#version 410 core\n"
                         "in vec3 _POSITION;\n"
                         "in vec2 _TEXCOORD0;\n"
                         "\n"
                         "out vec2 vTexCoord;\n"
                         "\n"
                         "layout(std140) uniform CameraParams {\n"
                         "  uniform mat4 model;\n"
                         "  uniform mat4 view;\n"
                         "  uniform mat4 projection;\n"
                         "};\n"
                         "\n"
                         "void main()\n"
                         "{\n"
                         "  gl_Position = projection * view * model * vec4(_POSITION, 1.0);\n"
                         "  vTexCoord = _TEXCOORD0;\n"
                         "}";
  char fragment_shader[] =  "#version 410 core\n"
                            "in vec2 vTexCoord;\n"
                            "\n"
                            "out vec4 FragColor;\n"
                            "\n"
                            "uniform sampler2D Texture1;\n"
                            "uniform sampler2D Texture2;\n"
                            "\n"
                            "void main()\n"
                            "{\n"
                            "  FragColor = mix(texture(Texture1, vTexCoord), texture(Texture2, vTexCoord), 0.7);\n"
                            "}";
  {
    gfx::shader_binding_desc_gl bindings[] = {
        {.name = "Texture1",     .type = gfx::uniform_type::SAMPLER, .binding = 0},
        {.name = "Texture2",     .type = gfx::uniform_type::SAMPLER, .binding = 1},
        {.name = "CameraParams", .type = gfx::uniform_type::BUFFER,  .binding = 2},
    };

    gfx::shader_blob_header_gl header {
        .binding_table_offset = 0,
        .binding_table_size   = 3,
    };

    std::stringstream blob;
    blob.write((char*) &bindings, sizeof(bindings));

    char mem[sizeof(gfx::shader_blob_header_gl) + 3 * sizeof(gfx::shader_binding_desc_gl)];
    new (mem) gfx::shader_binding_desc_gl[] {
      { .name = "Texture1",     .type = gfx::uniform_type::SAMPLER, .binding = 0 },
      { .name = "Texture2",     .type = gfx::uniform_type::SAMPLER, .binding = 1 },
      { .name = "CameraParams", .type = gfx::uniform_type::BUFFER,  .binding = 2 },
    };

  }

  std::stringstream meta;
  char uniform1_str[] = "Texture1";
  char uniform2_str[] = "Texture2";
  char uniform3_str[] = "CameraParams";
  gfx::uniform_type::type sampler = gfx::uniform_type::SAMPLER;
  gfx::uniform_type::type buffer = gfx::uniform_type::BUFFER;

  uint8_t binding = 0;
  meta.write(uniform1_str, sizeof(uniform1_str));
  meta.write((char*) &sampler, sizeof(uint8_t));
  meta.write((char*) &binding, sizeof(uint8_t));

  binding = 1;
  meta.write(uniform2_str, sizeof(uniform2_str));
  meta.write((char*) &sampler, sizeof(uint8_t));
  meta.write((char*) &binding, sizeof(uint8_t));

  binding = 2;
  meta.write(uniform3_str, sizeof(uniform3_str));
  meta.write((char*) &buffer, sizeof(uint8_t));
  meta.write((char*) &binding, sizeof(uint8_t));

  uint32_t meta_size = meta.rdbuf()->pubseekoff(0, std::ios::end, std::ios_base::in);
  char metadata[meta_size];
  meta.rdbuf()->pubseekpos(0, std::ios_base::in);
  meta.rdbuf()->sgetn(metadata, meta_size);

  gfx::shader_handle shader = resource_command_buffer->create_shader({
    .vertex_shader = { .data = vertex_shader, .size = sizeof(vertex_shader) },
    .fragment_shader = { .data = fragment_shader, .size = sizeof(fragment_shader) },
  });

  static float size = 1.0f;
  static float hs = size * 0.5f;
  static float vertices[] = {
      // pos          // tex coords
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

  gfx::memory vertex_mem;
  gfx::vertexbuf_handle vertexbuf = resource_command_buffer->create_vertex_buffer(
      gfx::vertex_layout_desc()
          .add(gfx::vertex_semantic::POSITION, gfx::vertex_type::FLOAT, 3)
          .add(gfx::vertex_semantic::TEXCOORD0, gfx::vertex_type::FLOAT, 2),
      sizeof(vertices),
      vertex_mem
  );
  std::memcpy(vertex_mem.data, vertices, sizeof(vertices));

  gfx::uniform_handle uniform = resource_command_buffer->create_uniform(
  {
          { .type = gfx::uniform_type::SAMPLER, .binding = 0 },
          { .type = gfx::uniform_type::SAMPLER, .binding = 1 },
          { .type = gfx::uniform_type::BUFFER,  .binding = 2 },
      });
  resource_command_buffer->update_uniform(uniform, 0, texture0);
  resource_command_buffer->update_uniform(uniform, 1, texture1);

  gfx::swap_chain swap_chain = renderer.create_swap_chain(window.get_handle());

  renderer.submit(resource_command_buffer);

  const vec4 normalized_rect { 0.0f, 0.0f, 1.0f, 1.0f };
  const float fov = 60.0f;
  const float near = 0.1f;
  const float far = 1000.0f;
  vec2i resolution = window.get_resolution();
  vec4 viewport { };
  viewport.z = (float) resolution.x;
  viewport.w = (float) resolution.y;
  float ratio = viewport.z / viewport.w;

  struct camera_model_view_projection {
    mat4 model;
    mat4 view;
    mat4 projection;
  } mvp;

  mvp.projection = mat4::perspective(
      fov * math::DEG_TO_RAD, ratio * normalized_rect.z / normalized_rect.w, near, far
  );

  mvp.view = mat4::inverse(
      mat4::translation({0, 0, -5})
  );


  bool running = true;
  while (running) {

    window.update();

    window_event event;
    while (window.poll_event(event)) {
      if (event.type == event_type::Close) {
        running = false;
      }
    }

    renderer.begin_frame();

    gfx::render_command_buffer* render_buffer = renderer.create_render_command_buffer();
    gfx::resource_command_buffer* resource_buffer = renderer.create_resource_command_buffer();

    float time = glfwGetTime();

    quat rot = quat::axis(vec3::normalized({1, 1, 1}), time);
    mvp.model = mat4::trs(vec3 { 0.0f, 0, 0 }, rot, {1, 1, 1});

    resource_buffer->update_uniform(uniform, 2, &mvp, sizeof(camera_model_view_projection));

    render_buffer->bind_render_pass(0, swap_chain.back_buffer_handle);
    render_buffer->draw({
        .sort_key = 0,
        .shader = shader,
        .vertexbuf = vertexbuf,
        .uniforms = { uniform }
    });

    renderer.submit(resource_buffer);
    renderer.submit(render_buffer);
    renderer.swap(swap_chain);
  }

}