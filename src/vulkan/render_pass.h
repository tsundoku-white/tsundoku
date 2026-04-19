#pragma once

#include "core/common.h"

namespace tsundoku
{
  class VkContext;
  class Swapchain;

  class RenderPass
  {
    public:
      RenderPass(VkContext &context, Swapchain &swapchain);
      ~RenderPass();

      RenderPass(const RenderPass&) = delete;
      RenderPass& operator=(const RenderPass&) = delete;

      VkRenderPass get_render_pass() const { return m_render_pass; }

    private:
      VkDevice     m_device      = VK_NULL_HANDLE;
      VkRenderPass m_render_pass = VK_NULL_HANDLE;
  };
}
