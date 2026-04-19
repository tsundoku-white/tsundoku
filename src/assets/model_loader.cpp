#include "assets/model_loader.h"
#include "core/common.h"
#include "vulkan/vk_context.h"
#include <string>
#include "vulkan/vertex_buffer.h"

#define CGLTF_IMPLEMENTATION
#include "cgltf.h"

namespace tsundoku
{
  Model_Loader::Model_Loader(VkContext &context, std::string path)
  {

    cgltf_options options = {};
    cgltf_data*   data    = nullptr;

    if (cgltf_parse_file(&options, path.c_str(), &data) != cgltf_result_success)
      LOG_ERR("failed to parse glTF file: {}", path.c_str());

    if (cgltf_load_buffers(&options, data, path.c_str()) != cgltf_result_success)
    {
      cgltf_free(data);
      LOG_ERR("failed to load glTF buffers");
    }

    m_data = data;

    std::vector<Vertex>   vertices;
    std::vector<uint32_t> indices;

    for (size_t i = 0; i < m_data->meshes_count; i++)
    {
      cgltf_mesh& mesh = m_data->meshes[i];

      for (size_t j = 0; j < mesh.primitives_count; j++)
      {
        cgltf_primitive& prim         = mesh.primitives[j];
        uint32_t         vertex_start = static_cast<uint32_t>(vertices.size());

        cgltf_accessor* pos_acc    = nullptr;
        cgltf_accessor* normal_acc = nullptr;
        cgltf_accessor* uv_acc     = nullptr;

        for (size_t k = 0; k < prim.attributes_count; k++)
        {
          cgltf_attribute& attr = prim.attributes[k];
          if (attr.type == cgltf_attribute_type_position) pos_acc    = attr.data;
          if (attr.type == cgltf_attribute_type_normal)   normal_acc = attr.data;
          if (attr.type == cgltf_attribute_type_texcoord) uv_acc     = attr.data;
        }

        if (!pos_acc) continue;

        for (size_t v = 0; v < pos_acc->count; v++)
        {
          Vertex vert{};

          float pos[3] = {};
          cgltf_accessor_read_float(pos_acc, v, pos, 3);
          vert.pos = { pos[0], pos[1], pos[2] };

          if (normal_acc)
          {
            float n[3] = {};
            cgltf_accessor_read_float(normal_acc, v, n, 3);
            vert.normal = { n[0], n[1], n[2] };
          }

          if (uv_acc)
          {
            float uv[2] = {};
            cgltf_accessor_read_float(uv_acc, v, uv, 2);
            vert.uv = { uv[0], uv[1] };
          }

          vertices.push_back(vert);
        }

        if (prim.indices)
        {
          for (size_t k = 0; k < prim.indices->count; k++)
            indices.push_back(vertex_start + cgltf_accessor_read_index(prim.indices, k));
        }
      }
    }
  }
    Model_Loader::~Model_Loader()
    {
      if (m_data)
      cgltf_free(m_data);
    }
  };
