#include "model.h"

#define CGLTF_IMPLEMENTATION
#include <cgltf.h>
#include <glm/gtc/matrix_transform.hpp>

namespace tsundoku
{
  Model::Model(std::string path)
  {
    load(path);
  }

  void Model::load(const std::string& path)
  {
    cgltf_options options = {};
    cgltf_data*   data    = nullptr;

    if (cgltf_parse_file(&options, path.c_str(), &data) != cgltf_result_success)
    {
      LOG_ERR("Failed to parse glTF file: {}", path);
      return;
    }

    if (cgltf_load_buffers(&options, data, path.c_str()) != cgltf_result_success)
    {
      LOG_ERR("Failed to load glTF buffers: {}", path);
      cgltf_free(data);
      return;
    }

    Mesh mesh;

    for (size_t m = 0; m < data->meshes_count; m++)
    {
      cgltf_mesh& cgltf_mesh = data->meshes[m];

      for (size_t p = 0; p < cgltf_mesh.primitives_count; p++)
      {
        cgltf_primitive& prim = cgltf_mesh.primitives[p];

        // ---- INDICES ----
        if (prim.indices)
        {
          size_t index_count = prim.indices->count;
          size_t base        = mesh.indices.size();
          mesh.indices.resize(base + index_count);
          cgltf_accessor_unpack_indices(prim.indices, mesh.indices.data() + base, sizeof(uint32_t), index_count);
        }

        // ---- VERTEX ATTRIBUTES ----
        size_t vertex_count = 0;
        for (size_t a = 0; a < prim.attributes_count; a++)
          if (prim.attributes[a].type == cgltf_attribute_type_position)
            vertex_count = prim.attributes[a].data->count;

        size_t base = mesh.vertices.size();
        mesh.vertices.resize(base + vertex_count);

        for (size_t a = 0; a < prim.attributes_count; a++)
        {
          cgltf_attribute& attr = prim.attributes[a];
          cgltf_accessor*  acc  = attr.data;

          switch (attr.type)
          {
            case cgltf_attribute_type_position:
              {
                for (size_t i = 0; i < vertex_count; i++)
                {
                  cgltf_accessor_read_float(acc, i, &mesh.vertices[base + i].position.x, 3);
                  mesh.vertices[base + i].color = {1.f, 1.f, 1.f};
                }
                break;
              }
            case cgltf_attribute_type_normal:
              {
                for (size_t i = 0; i < vertex_count; i++)
                  cgltf_accessor_read_float(acc, i, &mesh.vertices[base + i].normal.x, 3);
                break;
              }
            case cgltf_attribute_type_texcoord:
              {
                if (attr.index == 0) // TEXCOORD_0 only
                  for (size_t i = 0; i < vertex_count; i++)
                    cgltf_accessor_read_float(acc, i, &mesh.vertices[base + i].uv.x, 2);
                break;
              }
            default: break;
          }
        }
      }
    }

    cgltf_free(data);
    m_mesh = std::move(mesh);

    LOG_INFO("Loaded model: {} ({} vertices, {} indices)",
      path,
      m_mesh->vertices.size(),
      m_mesh->indices.size());
  }

  void Model::start()       { return; }
  void Model::update(float)
  {
    glm::vec3 world_pos = owner->transform.location + transform.location;
    glm::quat world_rot = owner->transform.rotation * transform.rotation;
    glm::vec3 world_scale = owner->transform.scale * transform.scale;

    glm::mat4 t = glm::translate(glm::mat4(1.f), world_pos);
    glm::mat4 r = glm::mat4_cast(world_rot);
    glm::mat4 s = glm::scale(glm::mat4(1.f), world_scale);

    model_matrix = t * r * s;
  }
  void Model::end()         { return; }
}
