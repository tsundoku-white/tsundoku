#include "vulkan/framebuffer.h"
#include "core/common.h"
#include "vulkan/swapchain.h"
#include "vulkan/vk_context.h"
#include <vulkan/vulkan_core.h>
#include "vulkan/render_pass.h"

namespace tsundoku
{
  Framebuffer::Framebuffer(VkContext &context, Swapchain &swapchain, RenderPass &render_pass)
  {
    m_device = context.get_device();
    m_render_pass = render_pass.get_render_pass();
    create(swapchain);
  }

  Framebuffer::~Framebuffer()
  {
    destroy();
  }

  void Framebuffer::destroy()
  {
    for (auto framebuffer : m_framebuffers)
    {
      vkDestroyFramebuffer(m_device, framebuffer, nullptr);
    }
    m_framebuffers.clear();
  }

  void Framebuffer::create(Swapchain &swapchain)
  {
    m_framebuffers.resize(swapchain.get_image_views().size());

    for (size_t i = 0; i < swapchain.get_image_views().size(); i++) {
      VkImageView attachments[] = {
        swapchain.get_image_views()[i]
      };

      VkFramebufferCreateInfo framebuffer_info{};
      framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
      framebuffer_info.renderPass = m_render_pass;
      framebuffer_info.attachmentCount = 1;
      framebuffer_info.pAttachments = attachments;
      framebuffer_info.width = swapchain.get_extent().width;
      framebuffer_info.height = swapchain.get_extent().height;
      framebuffer_info.layers = 1;

      if (vkCreateFramebuffer(m_device, &framebuffer_info,
            nullptr, &m_framebuffers[i]) != VK_SUCCESS)
      {
        LOG_ERR("[Framebuffer] failed to create framebuffer!");
      }
    }
  }
}
