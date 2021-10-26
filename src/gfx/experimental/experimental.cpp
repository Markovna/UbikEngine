#include <platform/window.h>
#include <gfx/experimental/gfx.h>
#include <gfx/experimental/renderer.h>
#include <gfx/experimental/render_context_opengl.h>
#include <gfx/experimental/vertex_layout_desc.h>
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

class a {
 public:
  auto begin() const { return values.begin(); }
  auto end() const { return values.end(); }

  void emplace(int val) { values.emplace_back(val); }
 private:
  std::vector<int> values;
};

class b : a {
 public:
  auto begin() const { return a::begin(); }
  auto end() const { return a::end(); }

  void push(int val) { emplace(val); }
};

int main(int argc, char* argv[]) {

  logger::init(std::filesystem::current_path().append("log").c_str());

  window window({512, 512});

  gfx::renderer renderer { gfx::render_context_opengl::create };

  gfx::resource_command_buffer* resource_command_buffer = renderer.create_resource_command_buffer();

  std::ifstream file0 { std::filesystem::current_path().append("assets/textures/container.jpg"), std::ios::in };
  gfx::texture_handle texture0 = load_texture(file0, resource_command_buffer);

  std::ifstream file1 { std::filesystem::current_path().append("assets/textures/seal.png"), std::ios::in };
  gfx::texture_handle texture1 = load_texture(file1, resource_command_buffer);


  char vertex_shader[] = "    in vec3 _POSITION;\n"
                         "    in vec2 _TEXCOORD0;\n"
                         "\n"
                         "    out vec2 vTexCoord;\n"
                         "\n"
                         "    layout(std140) uniform CameraParams {\n"
                         "      uniform mat4 model;\n"
                         "      uniform mat4 view;\n"
                         "      uniform mat4 projection;\n"
                         "    };\n"
                         "\n"
                         "    void main()\n"
                         "    {\n"
                         "        gl_Position = projection * view * model * vec4(_POSITION, 1.0);\n"
                         "        vTexCoord = _TEXCOORD0;\n"
                         "    }";
  char fragment_shader[] =  "    in vec2 vTexCoord;\n"
                            "\n"
                            "    out vec4 FragColor;\n"
                            "\n"
                            "    //uniform vec4 mainColor;\n"
                            "    uniform sampler2D Texture1;\n"
                            "    uniform sampler2D Texture2;\n"
                            "\n"
                            "    void main()\n"
                            "    {\n"
                            "      FragColor = mix(texture(Texture1, vTexCoord), texture(Texture2, vTexCoord), 0.7);\n"
                            "    }";

//  char metadata[] = "Texture1 1 0\n"
//                   "Texture2 1 1\n"
//                   "CameraParams 0 2";

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
    .metadata = { .data = metadata, .size = meta_size }
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
            { .binding = 0, .type = gfx::uniform_type::SAMPLER },
            { .binding = 1, .type = gfx::uniform_type::SAMPLER },
            { .binding = 2, .type = gfx::uniform_type::BUFFER },
        });
  resource_command_buffer->update_uniform(uniform, 0, texture0);
  resource_command_buffer->update_uniform(uniform, 1, texture1);

  gfx::swap_chain swap_chain = renderer.create_swap_chain(window.get_handle());

  renderer.submit(resource_command_buffer);

  struct camera_model_view_projection {
    mat4 model;
    mat4 view;
    mat4 projection;
  } mvp;

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

    const vec4 normalized_rect { 0.0f, 0.0f, 1.0f, 1.0f };
    const float fov = 60.0f;
    const float near = 0.1f;
    const float far = 1000.0f;
    vec2i resolution = window.get_resolution();
    vec4 viewport { };
    viewport.z = (float) resolution.x;
    viewport.w = (float) resolution.y;
    float ratio = viewport.z / viewport.w;

    mvp.projection = mat4::perspective(
        fov * math::DEG_TO_RAD, ratio * normalized_rect.z / normalized_rect.w, near, far
      );

    mvp.view = mat4::inverse(
        mat4::translation({0, 0, -5})
      );

    float time = glfwGetTime();

    quat rot = quat::axis(vec3::normalized({1, 1, 1}), time);
    mvp.model = mat4::trs(vec3 { 0.0f, 0, 0 }, rot, {1, 1, 1});

    resource_buffer->update_uniform(uniform, 2, &mvp, sizeof(camera_model_view_projection));

    render_buffer->bind_render_pass(0, swap_chain.back_buffer_handle);
    render_buffer->draw({
        .sort_key = 0,
        .shader = shader,
        .vertexbuf = vertexbuf,
        .uniforms = {uniform}
    });

    renderer.submit(resource_buffer);
    renderer.submit(render_buffer);
    renderer.swap(swap_chain);
  }

}