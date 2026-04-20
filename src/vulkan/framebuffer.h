#pragma once 

#include "core/common.h"


namespace tsundoku
{
  class VkContext;
  class Swapchain;
  class RenderPass;

  class Framebuffer 
  {
    public:
      Framebuffer(VkContext &context, Swapchain &swapchain, RenderPass &render_pass);
      ~Framebuffer();

      Framebuffer(const Framebuffer&) = delete;
      Framebuffer& operator = (const Framebuffer&) = delete;

      void destroy();
      void create(Swapchain &swapchain);

      VkRenderPass get_render_pass() const { return m_render_pass; }
      const std::vector<VkFramebuffer>& get_framebuffers() const { return m_framebuffers; }
    private:
      VkDevice         m_device           = VK_NULL_HANDLE;
      VkRenderPass     m_render_pass      = VK_NULL_HANDLE;
      VkImage        m_depth_image        = VK_NULL_HANDLE;
      VkDeviceMemory m_depth_memory       = VK_NULL_HANDLE;
      VkImageView    m_depth_image_view   = VK_NULL_HANDLE;
      VkPhysicalDevice m_physical_device  = VK_NULL_HANDLE;

      std::vector<VkFramebuffer> m_framebuffers = {};

      uint32_t find_memory_type(VkPhysicalDeviceMemoryProperties mem_properties, 
                            uint32_t type_filter, VkMemoryPropertyFlags properties);

  };
}

