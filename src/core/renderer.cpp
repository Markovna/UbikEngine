#include "core/components/mesh_component.h"
#include "core/assets/shader.h"
#include "renderer.h"
#include "world.h"
#include "gfx/gfx.h"

mat4 get_projection_matrix(const camera_component& camera, const float ratio) {
  const vec4 normalized_rect = camera.normalized_rect;
  const float fov = camera.fov;
  const float near = camera.near;
  const float far = camera.far;
  if (fov > 0.0f) {
    return mat4::perspective(
        fov * math::DEG_TO_RAD, ratio * normalized_rect.z / normalized_rect.w, near, far
    );
  }

  const float ortho_size = camera.orthogonal_size;
  return mat4::ortho(
      ratio * ortho_size * normalized_rect.z,
      ortho_size * normalized_rect.w,
      near,
      far);
}

void renderer::update_views(world* world, vec4 viewport, gfx::framebuf_handle fb_handle, const std::function<bool(entity, const camera_component&)>& predicate) {
  auto camera_view = world->view<camera_component>();
  for (ecs::entity camera_ent : camera_view) {
    camera_component &camera = camera_view.get(camera_ent);
    if (predicate && !predicate({camera_ent}, camera))
      continue;

    vec4 normalized_rect = camera.normalized_rect;

    gfx::set_view(camera.viewid, mat4::inverse((mat4) world->world_transform({camera_ent})));
    gfx::set_projection(camera.viewid, get_projection_matrix(camera, viewport.z / viewport.w));
    gfx::set_view_rect(camera.viewid, {0, 0, int(viewport.z * normalized_rect.z), int(viewport.w * normalized_rect.w)});
    gfx::set_view_buffer(camera.viewid, fb_handle);
    gfx::set_clear(camera.viewid, camera.clear_flags);
    gfx::set_clear_color(camera.viewid, camera.clear_color);
  }
}

void renderer::render(
    const mat4& model,
    gfx::shader_handle shader,
    gfx::vertexbuf_handle vb,
    gfx::indexbuf_handle ib,
    const camera_component& camera) {

  gfx::set_buffer(vb);
  gfx::set_buffer(ib);

  gfx::set_transform(model);

  gfx::render(camera.viewid, shader);
}

void renderer::render(world* world, const std::function<bool(entity, const camera_component&)>& predicate, resources::handle<shader> shader) {
  auto camera_view = world->view<camera_component>();
  auto mesh_view = world->view<mesh_component>();

  for (ecs::entity camera_ent : camera_view) {
    camera_component& camera = camera_view.get(camera_ent);
    if (predicate && !predicate({camera_ent}, camera))
      continue;

    for (ecs::entity mesh_id : mesh_view) {
      const mesh_component& mesh = mesh_view.get(mesh_id);
      const transform& model = world->world_transform({ mesh_id });

      gfx::set_uniform(mesh.GetMainTextureUniform(), mesh.GetMainTexture()->handle(), 1);
      gfx::set_uniform(mesh.GetSecondTextureUniform(), mesh.GetSecondTexture()->handle(), 0);
      gfx::set_uniform(mesh.GetColorUniform(), mesh.color());

      gfx::set_buffer(mesh.GetVertexBuf());
      gfx::set_buffer(mesh.GetIndexBuf());

      gfx::set_transform((mat4) model);

      gfx::render(camera.viewid, shader ? shader->handle() : mesh.GetShader()->handle());
   }

  }
}
