#include <base/color.h>
#include "dcc_asset.h"

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

#include "base/log.h"
#include "asset_repository.h"
#include "world.h"
#include "gfx/gfx.h"
#include "component_loader.h"
#include "assets_filesystem.h"
#include "texture_compiler.h"

asset& process_node(aiNode *node, const aiScene *scene, asset_array& all_meshes, asset_array& nodes, asset_repository& repository) {
  asset& node_asset = repository.create_asset();
  repository.push_back(nodes, node_asset);

  repository.set_value(node_asset, "name", node->mName.C_Str());

  if (node->mNumMeshes > 0) {
    asset_array &meshes = repository.create_array();
    repository.set_value(node_asset, "meshes", meshes);

    for (size_t i = 0; i < node->mNumMeshes; i++) {
      uint32_t mesh_index = node->mMeshes[i];
      const asset &mesh_asset = all_meshes[mesh_index];
      repository.push_back(meshes, repository.get_guid(mesh_asset).str());
    }
  }

  aiMatrix4x4 matrix = node->mTransformation;
  transform local = transform::from_matrix(
      { .data = {
          { matrix.a1, matrix.a2, matrix.a3, matrix.a4 },
          { matrix.b1, matrix.b2, matrix.b3, matrix.b4 },
          { matrix.c1, matrix.c2, matrix.c3, matrix.c4 },
          { matrix.d1, matrix.d2, matrix.d3, matrix.d4 }
      }}
  );

  asset& position = repository.create_asset();
  repository.set_value(node_asset, "position", position);
  repository.set_value(position, "x", local.position.x);
  repository.set_value(position, "y", local.position.y);
  repository.set_value(position, "z", local.position.z);

  asset& rotation = repository.create_asset();
  repository.set_value(node_asset, "rotation", rotation);
  repository.set_value(rotation, "x", local.rotation.x);
  repository.set_value(rotation, "y", local.rotation.y);
  repository.set_value(rotation, "z", local.rotation.z);
  repository.set_value(rotation, "w", local.rotation.w);

  asset& scale = repository.create_asset();
  repository.set_value(node_asset, "scale", scale);
  repository.set_value(scale, "x", local.scale.x);
  repository.set_value(scale, "y", local.scale.y);
  repository.set_value(scale, "z", local.scale.z);

  if (node->mNumChildren > 0) {
    asset_array& children = repository.create_array();
    repository.set_value(node_asset, "children", children);

    for (size_t i = 0; i < node->mNumChildren; i++) {
      auto& child_asset = process_node(node->mChildren[i], scene, all_meshes, nodes, repository);
      repository.push_back(children, repository.get_guid(child_asset).str());
    }
  }
  return node_asset;
}

static const char *GetShortFilename(const char *filename)
{
  const char *lastSlash = strrchr(filename, '/');
  if (lastSlash == 0) {
    lastSlash = strrchr(filename, '\\');
  }
  const char *shortFilename = lastSlash != 0 ? lastSlash + 1 : filename;
  return shortFilename;
}

//! Returns an embedded texture
const struct aiTexture *GetEmbeddedTexture(const struct aiScene *scene, const char *filename)
{
  const char *shortFilename = GetShortFilename(filename);
  for (unsigned int i = 0; i < scene->mNumTextures; i++) {
    const char *shortTextureFilename = GetShortFilename(scene->mTextures[i]->mFilename.data);
    if (strcmp(shortTextureFilename, shortFilename) == 0) {
      return scene->mTextures[i];
    }
  }
  return 0;
}

void extract_texture(
    const fs::path& asset_path,
    const aiScene* ai_scene,
    const aiMaterial *ai_material,
    aiTextureType ai_texture_type,
    asset_repository& repository,
    asset& material_asset,
    const asset::key_t& texture_key,
    asset_array& textures,
    asset_array& buffers,
    std::unordered_map<std::string, asset_id>& texture_path_to_id) {

  const uint32_t count = aiGetMaterialTextureCount(ai_material, ai_texture_type);
  if (!count)
    return;

  aiString ai_path;
  aiTextureMapping ai_mapping;
  uint32_t ai_uv_set, ai_flags;
  float ai_blend;
  aiTextureOp ai_op;
  aiTextureMapMode ai_map_mode;

  const aiReturn res = aiGetMaterialTexture(ai_material, ai_texture_type, 0, &ai_path, &ai_mapping, &ai_uv_set, &ai_blend, &ai_op, &ai_map_mode, &ai_flags);
  if (res != aiReturn_SUCCESS) {
    return;
  }

  if (!ai_path.length) {
    return;
  }

  asset_id& texture_asset_id = texture_path_to_id[ai_path.C_Str()];
  if (!texture_asset_id) {
    auto& texture_asset = repository.create_asset();
    repository.push_back(textures, texture_asset);
    const aiTexture *ai_texture = nullptr;
    if (ai_path.data[0] == '*') {
      uint32_t texture_idx = std::atoi(ai_path.data + 1);
      if (texture_idx < ai_scene->mNumTextures) {
        ai_texture = ai_scene->mTextures[texture_idx];
      }
    } else {
      ai_texture = GetEmbeddedTexture(ai_scene, ai_path.data);
    }

    if (ai_texture) {

      if (ai_texture->mHeight > 0) {
        repository.set_value(texture_asset, "type", (uint32_t) dcc_asset_texture_type::RAW);

        texture_data_desc desc {
          .width = ai_texture->mWidth,
          .height = ai_texture->mHeight,
          .format = texture_format::RGBA8
        };
        auto buf_id = repository.create_buffer(sizeof(desc) + texture_size(desc));
        repository.update_buffer(buf_id, 0, &desc, sizeof(desc));
        repository.update_buffer(buf_id, sizeof(desc), ai_texture->pcData, texture_size(desc));

        auto& buffer_asset = repository.create_asset();
        repository.set_value(buffer_asset, "data", buf_id);
        repository.push_back(buffers, buffer_asset);

        repository.set_value(texture_asset, "name", ai_texture->mFilename.C_Str());
        repository.set_ref(texture_asset, "buffer", buffer_asset.id());
      } else {
        // TODO:
      }

    } else {
      fs::path texture_path { fs::append(asset_path.parent_path(), ai_path.data) };
      if (fs::exists(texture_path)) {
        std::ifstream file(texture_path, std::ios::binary | std::ios::in);

        fs::path extension = texture_path.extension();
        dcc_asset_texture_type texture_type = dcc_asset_texture_type::UNKNOWN;
        if (extension == ".jpeg" || extension == ".jpg") {
          texture_type = dcc_asset_texture_type::JPEG;
        } else if (extension == ".png") {
          texture_type = dcc_asset_texture_type::PNG;
        } else if (extension == ".tiff") {
          texture_type = dcc_asset_texture_type::TIFF;
        } else if (extension == ".tga") {
          texture_type = dcc_asset_texture_type::TGA;
        }

        if (texture_type != dcc_asset_texture_type::UNKNOWN) {
          size_t buf_size = file.rdbuf()->pubseekoff(0, std::ios::end, std::ios::in);
          char buffer[buf_size];
          file.rdbuf()->pubseekpos(0, std::ios_base::in);
          file.rdbuf()->sgetn(buffer, buf_size);

          auto buf_id = repository.create_buffer(buf_size);
          repository.update_buffer(buf_id, 0, buffer, buf_size);

          auto &buffer_asset = repository.create_asset();
          repository.set_value(buffer_asset, "data", buf_id);
          repository.push_back(buffers, buffer_asset);

          repository.set_value(texture_asset, "name", texture_path.filename().c_str());
          repository.set_value(texture_asset, "type", (uint32_t) texture_type);
          repository.set_ref(texture_asset, "buffer", buffer_asset.id());
        }
      } else {
        logger::core::Warning("Extract texture during dcc_asset import failed: texture file at path {} doesn't exist.", texture_path.c_str());
      }

    }

    if (!texture_asset.contains("type")) {
      repository.set_value(texture_asset, "type", (uint32_t) dcc_asset_texture_type::UNKNOWN);
    }

    texture_asset_id = texture_asset.id();
  }

  repository.set_ref(material_asset, texture_key, texture_asset_id);
}

asset_id create_dcc_asset(const fs::path &path, asset_repository& repository) {
  Assimp::Importer importer;
  const aiScene *scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

  if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
    logger::core::Error("[create_dcc_asset] Error: {}", importer.GetErrorString());
    return { };
  }

  asset& root = repository.create_asset();

  asset_array& materials = repository.create_array();
  repository.set_value(root, "materials", materials);

  asset_array& textures = repository.create_array();
  repository.set_value(root, "textures", textures);

  asset_array& buffers = repository.create_array();
  repository.set_value(root, "buffers", buffers);

  std::unordered_map<std::string, asset_id> texture_path_to_id;

  for (size_t mat_i = 0; mat_i < scene->mNumMaterials; ++mat_i) {
    const aiMaterial *ai_material = scene->mMaterials[mat_i];

    asset& material_asset = repository.create_asset();
    repository.push_back(materials, material_asset);

    const aiString ai_name = ai_material->GetName();
    repository.set_value(material_asset, "name", ai_name.C_Str());

    int32_t double_sided = 0;
    aiGetMaterialIntegerArray(ai_material, AI_MATKEY_TWOSIDED, &double_sided, 0);

    repository.set_value(material_asset, "double_sided", (bool) double_sided);

    extract_texture(
        path,
        scene,
        ai_material,
        aiTextureType_DIFFUSE,
        repository,
        material_asset,
        "diffuse",
        textures,
        buffers,
        texture_path_to_id
      );

    extract_texture(
        path,
        scene,
        ai_material,
        aiTextureType_NORMALS,
        repository,
        material_asset,
        "normals",
        textures,
        buffers,
        texture_path_to_id
    );

    extract_texture(
        path,
        scene,
        ai_material,
        aiTextureType_EMISSIVE,
        repository,
        material_asset,
        "emissive",
        textures,
        buffers,
        texture_path_to_id
    );

  }

  asset_array& accessors = repository.create_array();
  repository.set_value(root, "accessors", accessors);

  asset_array& meshes = repository.create_array();
  repository.set_value(root, "meshes", meshes);

  for (size_t mesh_i = 0; mesh_i < scene->mNumMeshes; ++mesh_i) {

    aiMesh* mesh = scene->mMeshes[mesh_i];

    asset& mesh_asset = repository.create_asset();
    repository.push_back(meshes, mesh_asset);

    uint32_t mat_index = mesh->mMaterialIndex;
    const asset& material = materials[mat_index];
    repository.set_ref(mesh_asset, "material", material.id());

    asset_array& attributes = repository.create_array();
    repository.set_value(mesh_asset, "attributes", attributes);

    size_t v_buf_size = 0;
    asset& v_buf_asset = repository.create_asset();
    repository.push_back(buffers, v_buf_asset);

    size_t i_buf_size = 0;
    asset& i_buf_asset = repository.create_asset();
    repository.push_back(buffers, i_buf_asset);

    asset& position_accessor = repository.create_asset();
    repository.push_back(accessors, position_accessor);
    repository.set_value(position_accessor, "buffer", repository.get_guid(v_buf_asset).str());

    repository.set_value(position_accessor, "float", true);
    repository.set_value(position_accessor, "size", 4);
    repository.set_value(position_accessor, "components", 3);
    repository.set_value(position_accessor, "offset", v_buf_size);
    repository.set_value(position_accessor, "count", mesh->mNumVertices);
    v_buf_size += sizeof(aiVector3D) * mesh->mNumVertices;

    asset& position_attr = repository.create_asset();
    repository.push_back(attributes, position_attr);
    repository.set_value(position_attr, "accessor", repository.get_guid(position_accessor).str());
    repository.set_value(position_attr, "semantic", (int) vertex_semantic::POSITION);

    if (mesh->HasNormals()) {
      asset &normals_accessor = repository.create_asset();
      repository.push_back(accessors, normals_accessor);
      repository.set_value(normals_accessor, "buffer", repository.get_guid(v_buf_asset).str());
      repository.set_value(normals_accessor, "float", true);
      repository.set_value(normals_accessor, "size", 4);
      repository.set_value(normals_accessor, "components", 3);
      repository.set_value(normals_accessor, "offset", v_buf_size);
      repository.set_value(normals_accessor, "count", mesh->mNumVertices);
      v_buf_size += sizeof(aiVector3D) * mesh->mNumVertices;

      asset& normal_attr = repository.create_asset();
      repository.push_back(attributes, normal_attr);
      repository.set_value(normal_attr, "accessor", repository.get_guid(normals_accessor).str());
      repository.set_value(normal_attr, "semantic", (int) vertex_semantic::NORMAL);
    }

    if (mesh->HasTextureCoords(0)) {
      asset &tex_coord_accessor = repository.create_asset();
      repository.push_back(accessors, tex_coord_accessor);
      repository.set_value(tex_coord_accessor, "buffer",repository.get_guid(v_buf_asset).str());
      repository.set_value(tex_coord_accessor, "float", true);
      repository.set_value(tex_coord_accessor, "size", 4);
      repository.set_value(tex_coord_accessor, "components", 2);
      repository.set_value(tex_coord_accessor, "offset", v_buf_size);
      repository.set_value(tex_coord_accessor, "count", mesh->mNumVertices);
      v_buf_size += 2 * sizeof(ai_real) * mesh->mNumVertices;

      asset& texcoord_attr = repository.create_asset();
      repository.push_back(attributes, texcoord_attr);
      repository.set_value(texcoord_attr, "accessor", repository.get_guid(tex_coord_accessor).str());
      repository.set_value(texcoord_attr, "semantic", (int) vertex_semantic::TEXCOORD0);
    }

    if (mesh->HasTangentsAndBitangents()) {
      asset &tangent_accessor = repository.create_asset();
      repository.push_back(accessors, tangent_accessor);
      repository.set_value(tangent_accessor, "buffer", repository.get_guid(v_buf_asset).str());
      repository.set_value(tangent_accessor, "float", true);
      repository.set_value(tangent_accessor, "size", 4);
      repository.set_value(tangent_accessor, "components", 3);
      repository.set_value(tangent_accessor, "offset", v_buf_size);
      repository.set_value(tangent_accessor, "count", mesh->mNumVertices);
      v_buf_size += sizeof(aiVector3D) * mesh->mNumVertices;

      asset& tangent_attr = repository.create_asset();
      repository.push_back(attributes, tangent_attr);
      repository.set_value(tangent_attr, "accessor", repository.get_guid(tangent_accessor).str());
      repository.set_value(tangent_attr, "semantic", (int) vertex_semantic::TANGENT);
    }

    asset &index_accessor = repository.create_asset();
    repository.push_back(accessors, index_accessor);
    repository.set_value(index_accessor, "buffer", repository.get_guid(i_buf_asset).str());
    repository.set_value(index_accessor, "float", false);
    repository.set_value(index_accessor, "size", 4);
    repository.set_value(index_accessor, "unsigned", true);
    repository.set_value(index_accessor, "components", 1);
    repository.set_value(index_accessor, "offset", i_buf_size);
    repository.set_value(index_accessor, "count", 3 * mesh->mNumFaces);
    i_buf_size += sizeof(uint32_t) * 3 *  mesh->mNumFaces;

    repository.set_value(mesh_asset, "indices", repository.get_guid(index_accessor).str());

    auto vbuf_id = repository.create_buffer(v_buf_size);
    repository.set_value(v_buf_asset, "data", vbuf_id);

    size_t vbuf_offset = 0;

    repository.update_buffer(vbuf_id, vbuf_offset, mesh->mVertices, sizeof(aiVector3D) * mesh->mNumVertices);
    vbuf_offset += sizeof(aiVector3D) * mesh->mNumVertices;

    if (mesh->HasNormals()) {
      repository.update_buffer(vbuf_id, vbuf_offset, mesh->mNormals, sizeof(aiVector3D) * mesh->mNumVertices);
      vbuf_offset += sizeof(aiVector3D) * mesh->mNumVertices;
    }

    if (mesh->HasTextureCoords(0)) {
      float tex_coord_buf[2 * mesh->mNumVertices];
      for (size_t vert = 0; vert < mesh->mNumVertices; ++vert) {
        tex_coord_buf[2 * vert + 0] = mesh->mTextureCoords[0][vert].x;
        tex_coord_buf[2 * vert + 1] = mesh->mTextureCoords[0][vert].y;
      }

      repository.update_buffer(vbuf_id, vbuf_offset, tex_coord_buf, sizeof(tex_coord_buf));
      vbuf_offset += sizeof(tex_coord_buf);
    }

    if (mesh->HasTangentsAndBitangents()) {
      repository.update_buffer(vbuf_id, vbuf_offset, mesh->mTangents, sizeof(aiVector3D) * mesh->mNumVertices);
      vbuf_offset += sizeof(aiVector3D) * mesh->mNumVertices;
    }

    auto ibuf_id = repository.create_buffer(i_buf_size);
    repository.set_value(i_buf_asset, "data", ibuf_id);

    size_t ibuf_offset = 0;

    for (size_t face_i = 0; face_i < mesh->mNumFaces; ++face_i) {
      aiFace& face = mesh->mFaces[face_i];
      repository.update_buffer(ibuf_id, ibuf_offset, face.mIndices, sizeof(uint32_t) * face.mNumIndices);
      ibuf_offset += sizeof(uint32_t) * face.mNumIndices;
    }
  }

  asset_array& nodes = repository.create_array();
  repository.set_value(root, "nodes", nodes);

  asset& root_node_asset = process_node(scene->mRootNode, scene, meshes, nodes, repository);

  repository.set_value(root, "scene_root", repository.get_guid(root_node_asset).str());
  return root.id();
}

asset* parse_node(const asset& node_asset, asset_repository& rep, assets_filesystem& filesystem, const fs::path& path, std::unordered_map<guid, guid>& dcc_texture_to_texture) {
  asset& entity_asset = rep.create_asset();
  rep.set_value(entity_asset, "__type", "entity");
  rep.set_value(entity_asset, "name", node_asset.at("name").get<std::string&>());

  asset& components = rep.create_asset();
  rep.set_value(entity_asset, "components", components);

  asset& transform_comp = rep.create_asset();
  rep.set_value(components, "transform_component", transform_comp);

  rep.copy_value(transform_comp, "position", node_asset.at("position"));
  rep.copy_value(transform_comp, "rotation", node_asset.at("rotation"));
  rep.copy_value(transform_comp, "scale", node_asset.at("scale"));

  asset_array& children = rep.create_array();
  rep.set_value(entity_asset, "children", children);

  if (node_asset.contains("meshes")) {
    for (const std::string& mesh_guid : node_asset.at("meshes").get<asset_array&>()) {
      asset& mesh_entity = rep.create_asset();
      asset& child_components = rep.create_asset();
      rep.set_value(mesh_entity, "components", child_components);
      rep.set_value(mesh_entity, "__type", "entity");

      asset& child_transform_comp = rep.create_asset();
      rep.set_value(child_components, "transform_component", child_transform_comp);

      asset& position = rep.create_asset();
      rep.set_value(position, "x", 0.0f);
      rep.set_value(position, "y", 0.0f);
      rep.set_value(position, "z", 0.0f);

      asset& rotation = rep.create_asset();
      rep.set_value(rotation, "x", 0.0f);
      rep.set_value(rotation, "y", 0.0f);
      rep.set_value(rotation, "z", 0.0f);
      rep.set_value(rotation, "w", 1.0f);

      asset& scale = rep.create_asset();
      rep.set_value(scale, "x", 1.0f);
      rep.set_value(scale, "y", 1.0f);
      rep.set_value(scale, "z", 1.0f);

      rep.set_value(child_transform_comp, "position", position);
      rep.set_value(child_transform_comp, "rotation", rotation);
      rep.set_value(child_transform_comp, "scale", scale);

      asset& child_mesh_comp = rep.create_asset();
      rep.set_value(child_components, "mesh_component", child_mesh_comp);

      if (const asset* mesh_asset = rep.get_asset(guid::from_string(mesh_guid))) {
        guid material_guid = guid::from_string(mesh_asset->at("material"));
        if (asset* material_asset = rep.get_asset(material_guid); material_asset && material_asset->contains("diffuse")) {
          guid diffuse_dcc_texture_guid = guid::from_string(material_asset->at("diffuse"));
          guid& texture_guid = dcc_texture_to_texture[diffuse_dcc_texture_guid];
          if (!texture_guid.is_valid()) {
            if (asset *texture_asset = rep.get_asset(diffuse_dcc_texture_guid)) {
              const std::string& name = texture_asset->at("name");
              fs::path texture_path = fs::append(path.parent_path(), fs::concat(name, ".texture"));

              if (asset* texture = create_texture_from_dcc_asset(
                  *texture_asset,
                  texture_path,
                  rep,
                  filesystem
              )) {
                const asset &texture_output = texture->at("output");
                texture_guid = rep.get_guid(texture_output.id());
              }
            }
          }
          if (texture_guid.is_valid())
            rep.set_value(child_mesh_comp, "texture", texture_guid);
        }
      }

      rep.set_value(child_mesh_comp, "mesh", mesh_guid);

      rep.push_back(children, mesh_entity);

    }
  }

  if (node_asset.contains("children")) {
    for (const std::string& guid_str : node_asset.at("children").get<asset_array&>()) {
      asset* child_node_asset = rep.get_asset(guid::from_string(guid_str));
      asset* child_entity_asset = parse_node(*child_node_asset, rep, filesystem, path, dcc_texture_to_texture);
      rep.push_back(children, *child_entity_asset);
    }
  }

  if (children.empty()) {
    rep.erase(entity_asset, "children");
  }

  return &entity_asset;
}

asset_id create_entity_from_dcc_asset(const asset& dcc_asset, asset_repository& rep, assets_filesystem& filesystem) {
  auto& path = rep.get_asset_path(dcc_asset.id());

  std::unordered_map<guid, guid> cache;
  asset* root_asset = parse_node(
      *rep.get_asset(guid::from_string(dcc_asset.at("scene_root"))), rep, filesystem, path, cache);

  return root_asset->id();
}