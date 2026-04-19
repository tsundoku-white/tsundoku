#pragma once 

#include "core/common.h"
#include <glm/glm.hpp>
#include <vulkan/vulkan_core.h>

namespace tsundoku
{
  class VkContext;

  struct Vertex 
  {
    glm::vec3 pos;
    glm::vec3 color;
    glm::vec3 normal;
    glm::vec2 uv;

    static VkVertexInputBindingDescription getBindingDescription() {
      VkVertexInputBindingDescription bindingDescription{};
      bindingDescription.binding   = 0;
      bindingDescription.stride    = sizeof(Vertex);
      bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
      return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions() {
      std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};
      attributeDescriptions[0].binding  = 0;
      attributeDescriptions[0].location = 0;
      attributeDescriptions[0].format   = VK_FORMAT_R32G32B32_SFLOAT;
      attributeDescriptions[0].offset   = offsetof(Vertex, pos);
      attributeDescriptions[1].binding  = 0;
      attributeDescriptions[1].location = 1;
      attributeDescriptions[1].format   = VK_FORMAT_R32G32B32_SFLOAT;
      attributeDescriptions[1].offset   = offsetof(Vertex, color);
      return attributeDescriptions;
    }
  };

  class VertexBuffer 
  {
    public:
      VertexBuffer(VkContext& context);
      ~VertexBuffer();

      VertexBuffer(const VertexBuffer&)            = delete;
      VertexBuffer& operator=(const VertexBuffer&) = delete;

      void upload_vertices(const std::vector<Vertex>& vertices);
      void upload_indices(const std::vector<uint32_t>& indices);

      void bind_vertex(VkCommandBuffer cmd);
      void bind_index(VkCommandBuffer cmd);

      uint32_t get_index_count() const { return m_index_count; }

    private:
      VkDevice         m_device                = VK_NULL_HANDLE;
      VkPhysicalDevice m_physical              = VK_NULL_HANDLE;
      VkQueue          m_graphics_queue        = VK_NULL_HANDLE;
      uint32_t         m_queue_family          = 0;

      VkBuffer         m_vertex_buffer         = VK_NULL_HANDLE;
      VkDeviceMemory   m_vertex_buffer_memory  = VK_NULL_HANDLE;

      VkBuffer         m_index_buffer          = VK_NULL_HANDLE;
      VkDeviceMemory   m_index_buffer_memory   = VK_NULL_HANDLE;
      uint32_t         m_index_count           = 0;

      void create_buffer(VkDeviceSize size, VkBufferUsageFlags usage,
                         VkMemoryPropertyFlags props,
                         VkBuffer& buffer, VkDeviceMemory& memory);
      void copy_buffer(VkBuffer src, VkBuffer dst, VkDeviceSize size);
      uint32_t find_memory_type(uint32_t type_filter, VkMemoryPropertyFlags props);
  };
}
