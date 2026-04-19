#pragma once

#include "core/common.h"
#include <vulkan/vulkan_core.h>

namespace tsundoku
{
  class Platform;
  class VkContext;
  class Swapchain;
  class Camera;

  class CameraVulkan
  {
    public:
      CameraVulkan(Camera &camera, VkContext &context, Platform &platform, Swapchain &swapchain);
      ~CameraVulkan();

      CameraVulkan(const CameraVulkan&) = delete;
      CameraVulkan& operator = (const CameraVulkan&) = delete;

      void draw(uint32_t current_frame);

      VkDescriptorSet       get_descriptor_set(uint32_t frame) const { return m_frames[frame].descriptor; }
      VkDescriptorSetLayout get_descriptor_set_layout()        const { return m_descriptor_layout; }
    private:
      struct FrameData
      {
        VkBuffer        buffer     = VK_NULL_HANDLE;
        VkDeviceMemory  memory     = VK_NULL_HANDLE;
        VkDescriptorSet descriptor = VK_NULL_HANDLE;
        void*           mapped     = nullptr;
      };
      struct UBO 
      {
        glm::mat4 view;
        glm::mat4 proj;
      };

      std::array<FrameData, FRAMES_IN_FLIGHT> m_frames;

      VkDescriptorPool      m_descriptor_pool   = VK_NULL_HANDLE;
      VkDescriptorSetLayout m_descriptor_layout = VK_NULL_HANDLE;
      VkDevice              m_device            = VK_NULL_HANDLE;
      VkPhysicalDevice      m_physical_device   = VK_NULL_HANDLE;
 
      Camera*    m_camera   = nullptr;
      Swapchain* m_swapchain = nullptr;
 
      void create_descriptor_layout();
      void create_descriptor_pool();
      void create_buffers();
      void create_descriptor_sets();
      void update_projection();
  };
}
