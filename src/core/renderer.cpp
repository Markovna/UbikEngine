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

void renderer::render(world* world, vec4 viewport, gfx::framebuf_handle fb_handle, camera_component::kind_t camera_kind) {
  auto camera_view = world->view<camera_component>();
  auto mesh_view = world->view<mesh_component>();

  for (ecs::entity camera_ent : camera_view) {
    camera_component& camera = camera_view.get(camera_ent);
    if (camera.kind != camera_kind)
      continue;

    vec4 normalized_rect = camera.normalized_rect;

    gfx::camera_id camera_id = camera.idx;
    gfx::set_view(camera_id, mat4::inverse((mat4) world->world_transform({ camera_ent })));
    gfx::set_projection(camera_id, get_projection_matrix(camera, viewport.z / viewport.w));
    gfx::set_view_rect(camera_id,{ 0, 0, int(viewport.z * normalized_rect.z), int(viewport.w * normalized_rect.w)});
    gfx::set_view_buffer(camera_id, fb_handle);
    gfx::set_clear(camera_id, camera.clear_flags);
    gfx::set_clear_color(camera_id, camera.clear_color);

    for (ecs::entity mesh_id : mesh_view) {
      const mesh_component& mesh = mesh_view.get(mesh_id);
      const transform& model = world->world_transform({ mesh_id });

      gfx::set_uniform(mesh.GetMainTextureUniform(), mesh.GetMainTexture()->handle(), 0);
      gfx::set_uniform(mesh.GetSecondTextureUniform(), mesh.GetSecondTexture()->handle(), 1);
      gfx::set_uniform(mesh.GetColorUniform(), color::white());

      gfx::set_transform((mat4) model);
      gfx::set_buffer(mesh.GetVertexBuf());
      gfx::set_buffer(mesh.GetIndexBuf());

      gfx::render(camera_id, mesh.GetShader()->handle());
   }
  }
}
