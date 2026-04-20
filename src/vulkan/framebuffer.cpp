#include "vulkan/framebuffer.h"
#include "core/common.h"
#include "vulkan/swapchain.h"
#include "vulkan/vk_context.h"
#include "vulkan/render_pass.h"

namespace tsundoku
{
  Framebuffer::Framebuffer(VkContext &context, Swapchain &swapchain, RenderPass &render_pass)
    : m_device(context.get_device())
    , m_physical_device(context.get_physical_device())
    , m_render_pass(render_pass.get_render_pass())
  {
    create(swapchain);
  }

  Framebuffer::~Framebuffer()
  {
    destroy();
  }

void Framebuffer::destroy()
{
    // Destroy framebuffers
    for (auto framebuffer : m_framebuffers)
    {
        if (framebuffer != VK_NULL_HANDLE)
        {
            vkDestroyFramebuffer(m_device, framebuffer, nullptr);
            framebuffer = VK_NULL_HANDLE;  // Reset after destruction
        }
    }
    m_framebuffers.clear();

    // Destroy depth image view
    if (m_depth_image_view != VK_NULL_HANDLE)
    {
        vkDestroyImageView(m_device, m_depth_image_view, nullptr);
        m_depth_image_view = VK_NULL_HANDLE;  // IMPORTANT: Reset to NULL
    }

    // Destroy depth image
    if (m_depth_image != VK_NULL_HANDLE)
    {
        vkDestroyImage(m_device, m_depth_image, nullptr);
        m_depth_image = VK_NULL_HANDLE;  // IMPORTANT: Reset to NULL
    }

    // Free depth memory
    if (m_depth_memory != VK_NULL_HANDLE)
    {
        vkFreeMemory(m_device, m_depth_memory, nullptr);
        m_depth_memory = VK_NULL_HANDLE;  // IMPORTANT: Reset to NULL
    }
}
  void Framebuffer::create(Swapchain &swapchain)
  {
    destroy();

    // Create depth resources
    VkExtent2D swapchain_extent = swapchain.get_extent();
    
    VkImageCreateInfo image_info{};
    image_info.sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_info.imageType     = VK_IMAGE_TYPE_2D;
    image_info.extent.width  = swapchain_extent.width;
    image_info.extent.height = swapchain_extent.height;
    image_info.extent.depth  = 1;
    image_info.mipLevels     = 1;
    image_info.arrayLayers   = 1;
    image_info.format        = VK_FORMAT_D32_SFLOAT;
    image_info.tiling        = VK_IMAGE_TILING_OPTIMAL;
    image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_info.usage         = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    image_info.samples       = VK_SAMPLE_COUNT_1_BIT;
    image_info.sharingMode   = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateImage(m_device, &image_info, nullptr, &m_depth_image) != VK_SUCCESS)
      LOG_ERR("[Framebuffer] failed to create depth image");

    VkMemoryRequirements mem_requirements;
    vkGetImageMemoryRequirements(m_device, m_depth_image, &mem_requirements);

    VkPhysicalDeviceMemoryProperties mem_properties;
    vkGetPhysicalDeviceMemoryProperties(m_physical_device, &mem_properties);

    VkMemoryAllocateInfo alloc_info{};
    alloc_info.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.allocationSize  = mem_requirements.size;
    alloc_info.memoryTypeIndex = find_memory_type(mem_properties, mem_requirements.memoryTypeBits, 
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    if (vkAllocateMemory(m_device, &alloc_info, nullptr, &m_depth_memory) != VK_SUCCESS)
      LOG_ERR("[Framebuffer] failed to allocate depth image memory");

    vkBindImageMemory(m_device, m_depth_image, m_depth_memory, 0);

    // Create depth image view
    VkImageViewCreateInfo view_info{};
    view_info.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    view_info.image                           = m_depth_image;
    view_info.viewType                        = VK_IMAGE_VIEW_TYPE_2D;
    view_info.format                          = VK_FORMAT_D32_SFLOAT;
    view_info.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_DEPTH_BIT;
    view_info.subresourceRange.baseMipLevel   = 0;
    view_info.subresourceRange.levelCount     = 1;
    view_info.subresourceRange.baseArrayLayer = 0;
    view_info.subresourceRange.layerCount     = 1;

    if (vkCreateImageView(m_device, &view_info, nullptr, &m_depth_image_view) != VK_SUCCESS)
      LOG_ERR("[Framebuffer] failed to create depth image view");

    // Create framebuffers for each swapchain image
    const auto& swapchain_image_views = swapchain.get_image_views();
    m_framebuffers.resize(swapchain_image_views.size());

    for (size_t i = 0; i < swapchain_image_views.size(); i++)
    {
      VkImageView attachments[] = { swapchain_image_views[i], m_depth_image_view };

      VkFramebufferCreateInfo framebuffer_info{};
      framebuffer_info.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
      framebuffer_info.renderPass      = m_render_pass;
      framebuffer_info.attachmentCount = 2;
      framebuffer_info.pAttachments    = attachments;
      framebuffer_info.width           = swapchain_extent.width;
      framebuffer_info.height          = swapchain_extent.height;
      framebuffer_info.layers          = 1;

      if (vkCreateFramebuffer(m_device, &framebuffer_info, nullptr, &m_framebuffers[i]) != VK_SUCCESS)
        LOG_ERR("[Framebuffer] failed to create framebuffer");
    }

    LOG_PASS("[Framebuffer] created with depth testing support");
  }

  uint32_t Framebuffer::find_memory_type(VkPhysicalDeviceMemoryProperties mem_properties,
      uint32_t type_filter,
      VkMemoryPropertyFlags properties)
  {
    for (uint32_t i = 0; i < mem_properties.memoryTypeCount; i++)
    {
      if ((type_filter & (1 << i)) && 
          (mem_properties.memoryTypes[i].propertyFlags & properties) == properties)
      {
        return i;
      }
    }
    LOG_ERR("[Framebuffer] failed to find suitable memory type");
    return 0;
  }
}
