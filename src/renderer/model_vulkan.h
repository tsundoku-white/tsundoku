#pragma once

#include "core/common.h"
#include "vulkan/buffer.h"

namespace tsundoku
{
  class VkContext;
  class Swapchain;
  class Model;

  class ModelVulkan
  {
    public:
      ModelVulkan(Model& model, VkContext& context, Swapchain& swapchain);
      ~ModelVulkan();

      ModelVulkan(const ModelVulkan&)            = delete;
      ModelVulkan& operator=(const ModelVulkan&) = delete;

      // Called once per frame to update the model matrix UBO and bind/draw
      void draw(VkCommandBuffer cmd, VkPipelineLayout layout, uint32_t current_frame);

      VkDescriptorSet       get_descriptor_set(uint32_t frame) const { return m_frames[frame].descriptor; }
      VkDescriptorSetLayout get_descriptor_set_layout()        const { return m_descriptor_layout; }

      // Used by Pipeline at construction time — no instance needed
      static VkDescriptorSetLayout create_layout(VkDevice device);

    private:
      struct FrameData
      {
        Buffer          ubo_buffer;
        VkDescriptorSet descriptor = VK_NULL_HANDLE;
        void*           mapped     = nullptr;
      };

      struct UBO
      {
        glm::mat4 model;
      };

      std::array<FrameData, FRAMES_IN_FLIGHT> m_frames;

      Buffer   m_vertex_buffer;
      Buffer   m_index_buffer;
      uint32_t m_index_count = 0;

      VkDescriptorPool      m_descriptor_pool   = VK_NULL_HANDLE;
      VkDescriptorSetLayout m_descriptor_layout = VK_NULL_HANDLE;
      VkDevice              m_device            = VK_NULL_HANDLE;
      VkPhysicalDevice      m_physical_device   = VK_NULL_HANDLE;

      Model*     m_model     = nullptr;
      Swapchain* m_swapchain = nullptr;

      void create_descriptor_layout();
      void create_descriptor_pool();
      void upload_mesh(VkContext& context);
      void create_descriptor_sets();
  };
}
