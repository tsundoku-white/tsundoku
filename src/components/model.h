#pragma once

#include "core/common.h"
#include "core/entity.h"
#include <vulkan/vulkan_core.h>
#include <cstdint>
#include <optional>

namespace tsundoku
{
  class Model : public Component
  {
    public:
      struct Vertex
      {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec3 color = {1.f, 1.f, 1.f};
        glm::vec2 uv;

        static VkVertexInputBindingDescription getBindingDescription()
        {
          VkVertexInputBindingDescription desc{};
          desc.binding   = 0;
          desc.stride    = sizeof(Vertex);
          desc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
          return desc;
        }

        static std::array<VkVertexInputAttributeDescription, 4> getAttributeDescriptions()
        {
          std::array<VkVertexInputAttributeDescription, 4> descs{};

          descs[0].binding  = 0;
          descs[0].location = 0;
          descs[0].format   = VK_FORMAT_R32G32B32_SFLOAT;
          descs[0].offset   = offsetof(Vertex, position);

          descs[1].binding  = 0;
          descs[1].location = 1;
          descs[1].format   = VK_FORMAT_R32G32B32_SFLOAT;
          descs[1].offset   = offsetof(Vertex, normal);

          descs[2].binding  = 0;
          descs[2].location = 2;
          descs[2].format   = VK_FORMAT_R32G32B32_SFLOAT;
          descs[2].offset   = offsetof(Vertex, color);

          descs[3].binding  = 0;
          descs[3].location = 3;
          descs[3].format   = VK_FORMAT_R32G32B32A32_SFLOAT;
          descs[3].offset   = offsetof(Vertex, uv);

          return descs;
        }
      };

      struct Mesh
      {
        std::vector<Vertex>   vertices;
        std::vector<uint32_t> indices;
      };

    public:
      Model(std::string path);
      ~Model() = default;

      Model(const Model&)            = delete;
      Model& operator=(const Model&) = delete;

      const Mesh* get_mesh() const { return m_mesh.has_value() ? &m_mesh.value() : nullptr; }

      Transform transform;
      uint32_t  level_of_detail = 0;

      glm::mat4 model_matrix = glm::mat4(1.0f);;

    private:
      std::optional<Mesh> m_mesh;

      void load(const std::string& path);

      void update(float delta_time) override;
      void start()                  override;
      void end()                    override;
  };
}
