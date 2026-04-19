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
      VkDevice         m_device            = VK_NULL_HANDLE;
      VkRenderPass     m_render_pass       = VK_NULL_HANDLE;

      std::vector<VkFramebuffer> m_framebuffers = {};

  };
}

