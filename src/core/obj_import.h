#pragma once

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

#include "core/components/mesh_component.h"
#include "base/log.h"
#include "platform/file_system.h"
#include "world.h"
#include "renderer.h"
#include "gfx/vertex_layout_desc.h"
#include "viewer.h"

void process_mesh(aiMesh* mesh, const aiScene *scene, world& w, entity e, renderer& renderer) {
  auto cmd_buf = renderer.create_resource_command_buffer();
  auto& comp = w.emplace<mesh_component>(e.id);

  memory vertex_mem;
  comp.vb = cmd_buf->create_vertex_buffer(
      vertex_layout_desc()
        .add(vertex_semantic::POSITION, vertex_type::FLOAT, 3)
        .add(vertex_semantic::NORMAL, vertex_type::FLOAT, 3, true)
        .add(vertex_semantic::TEXCOORD0, vertex_type::FLOAT, 2)
        .add(vertex_semantic::TANGENT, vertex_type::FLOAT, 3),
      mesh->mNumVertices,
      vertex_mem
    );

  const size_t stride = 11 * sizeof(float);

  for (size_t i = 0; i < mesh->mNumVertices; i++) {
    std::memcpy(vertex_mem.data + stride * i, mesh->mVertices + i, sizeof(aiVector3D));
    if (mesh->HasNormals()) {
      std::memcpy(vertex_mem.data + stride * i + 3 * sizeof(float), mesh->mNormals + i, sizeof(aiVector3D));
    }

    if (mesh->mTextureCoords[0]) {
      std::memcpy(vertex_mem.data + stride * i + 6 * sizeof(float), mesh->mTextureCoords[0] + i, 2 * sizeof(ai_real));
      std::memcpy(vertex_mem.data + stride * i + 8 * sizeof(float), mesh->mTangents + i, sizeof(aiVector3D));
    } else {
      std::memset(vertex_mem.data + stride * i + 6 * sizeof(float), 0, 2 * sizeof(ai_real));
      std::memset(vertex_mem.data + stride * i + 8 * sizeof(float), 0, sizeof(aiVector3D));
    }
  }

  std::vector<uint32_t> indices;
  for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
    aiFace face = mesh->mFaces[i];

    for (size_t j = 0; j < face.mNumIndices; j++) {
      indices.push_back(face.mIndices[j]);
    }
  }

  memory index_mem;
  comp.ib = cmd_buf->create_index_buffer(indices.size(), index_mem);
  std::memcpy(index_mem.data, indices.data(), 2 * sizeof(ai_real));

  comp.model_buffer = cmd_buf->create_uniform_buffer(sizeof(mat4));
  comp.camera_buffer = cmd_buf->create_uniform_buffer(sizeof(view_projection));

  comp.uniform = cmd_buf->create_uniform(
      {
          { .type = uniform_type::SAMPLER, .binding = 0 },
          { .type = uniform_type::SAMPLER, .binding = 1 },
          { .type = uniform_type::BUFFER,  .binding = 0 },
          { .type = uniform_type::BUFFER,  .binding = 1 },
      });

  cmd_buf->set_uniform(comp.uniform, 0, comp.camera_buffer);
  cmd_buf->set_uniform(comp.uniform, 1, comp.model_buffer);

  renderer.submit(*cmd_buf);
}

void process_node(aiNode *node, const aiScene *scene, world& w, entity e, renderer& renderer) {
  for (size_t i = 0; i < node->mNumMeshes; i++) {
    aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
    process_mesh(mesh, scene, w, e, renderer);
  }

  for (size_t i = 0; i < node->mNumChildren; i++) {
    aiMatrix4x4 matrix = node->mChildren[i]->mTransformation;
    transform local = transform::from_matrix(
           { .data = {
              { matrix.a1, matrix.a2, matrix.a3, matrix.a4 },
              { matrix.b1, matrix.b2, matrix.b3, matrix.b4 },
              { matrix.c1, matrix.c2, matrix.c3, matrix.c4 },
              { matrix.d1, matrix.d2, matrix.d3, matrix.d4 }
          }}
        );
    auto child = w.create_entity(local, e);
    process_node(node->mChildren[i], scene, w, child, renderer);
  }
}

void import_obj(world& w, entity e, renderer& renderer, const fs::path& path) {
  Assimp::Importer importer;
  const aiScene *scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

  if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
    logger::core::Error("[import_obj] Error: {}", importer.GetErrorString());
    return;
  }

  process_node(scene->mRootNode, scene, w, e, renderer);
}