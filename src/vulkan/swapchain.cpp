#include "vulkan/swapchain.h"
#include "core/common.h"
#include "core/platform.h"
#include "vulkan/vk_context.h"

#define GLFW_INCLUDE_NONE 
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace tsundoku
{
  VkSurfaceFormatKHR Swapchain::choose_surface_format(const std::vector<VkSurfaceFormatKHR>& formats)
  {
    for (const auto& fmt : formats)
      if (fmt.format == VK_FORMAT_B8G8R8A8_SRGB &&
          fmt.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        return fmt;
    return formats[0];
  }

  VkPresentModeKHR Swapchain::choose_present_mode(const std::vector<VkPresentModeKHR>& modes)
  {
    if (!VSYNC)
    {
      for (const auto& mode : modes)
        if (mode == VK_PRESENT_MODE_MAILBOX_KHR)
          return mode;
    }
    return VK_PRESENT_MODE_FIFO_KHR;
  }

  VkExtent2D Swapchain::choose_extent(const VkSurfaceCapabilitiesKHR& caps)
  {
    if (caps.currentExtent.width != std::numeric_limits<uint32_t>::max())
      return caps.currentExtent;

    int w, h;
    glfwGetFramebufferSize(m_window, &w, &h);

    VkExtent2D extent = { static_cast<uint32_t>(w), static_cast<uint32_t>(h) };
    extent.width  = std::clamp(extent.width,  caps.minImageExtent.width,  caps.maxImageExtent.width);
    extent.height = std::clamp(extent.height, caps.minImageExtent.height, caps.maxImageExtent.height);
    return extent;
  }

  void Swapchain::create()
  {
    VkContext::SwapchainSupportDetails details;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_physical_device, m_surface, &details.capabilities);

    uint32_t format_count = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(m_physical_device, m_surface, &format_count, nullptr);
    if (format_count) {
      details.formats.resize(format_count);
      vkGetPhysicalDeviceSurfaceFormatsKHR(m_physical_device, m_surface, &format_count, details.formats.data());
    }

    uint32_t mode_count = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(m_physical_device, m_surface, &mode_count, nullptr);
    if (mode_count) {
      details.present_modes.resize(mode_count);
      vkGetPhysicalDeviceSurfacePresentModesKHR(m_physical_device, m_surface, &mode_count, details.present_modes.data());
    }

    VkSurfaceFormatKHR surface_format = choose_surface_format(details.formats);
    VkPresentModeKHR   present_mode   = choose_present_mode(details.present_modes);
    VkExtent2D         extent         = choose_extent(details.capabilities);

    uint32_t image_count = FRAMES_IN_FLIGHT;

    VkSwapchainCreateInfoKHR create_info{};
    create_info.sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    create_info.surface          = m_surface;
    create_info.minImageCount    = image_count;
    create_info.imageFormat      = surface_format.format;
    create_info.imageColorSpace  = surface_format.colorSpace;
    create_info.imageExtent      = extent;
    create_info.imageArrayLayers = 1;
    create_info.imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    create_info.oldSwapchain     = m_old_swapchain;
    create_info.presentMode      = present_mode;

    auto families = m_context->get_queue_families();
    uint32_t queue_family_indices[] = {
      families.graphics_family.value(),
      families.present_family.value()
    };

    if (families.graphics_family != families.present_family) {
      create_info.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;
      create_info.queueFamilyIndexCount = 2;
      create_info.pQueueFamilyIndices   = queue_family_indices;
    } else {
      create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    create_info.preTransform   = details.capabilities.currentTransform;
    create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    create_info.clipped        = VK_TRUE;

    if (vkCreateSwapchainKHR(m_device, &create_info, nullptr, &m_swapchain) != VK_SUCCESS)
      LOG_ERR("[Swapchain] failed to create swapchain");

    if (m_old_swapchain != VK_NULL_HANDLE) {
      vkDestroySwapchainKHR(m_device, m_old_swapchain, nullptr);
      m_old_swapchain = VK_NULL_HANDLE;
    }

    vkGetSwapchainImagesKHR(m_device, m_swapchain, &image_count, nullptr);
    m_images.resize(image_count);
    vkGetSwapchainImagesKHR(m_device, m_swapchain, &image_count, m_images.data());

    m_image_format = surface_format.format;
    m_extent       = extent;

    LOG_PASS("[Swapchain] created ({}x{}, {} images)", extent.width, extent.height, image_count);

    create_image_views();
  }

  void Swapchain::destroy()
  {
    if (m_device == VK_NULL_HANDLE) return;

    for (auto view : m_image_views)
      vkDestroyImageView(m_device, view, nullptr);

    m_image_views.clear();
    m_images.clear();

    if (m_swapchain != VK_NULL_HANDLE)
    {
      vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);
      m_swapchain = VK_NULL_HANDLE;
    }
  }

  Swapchain::Swapchain(Platform &platform, VkContext &context)
  {
    m_device          = context.get_device();
    m_physical_device = context.get_physical_device();
    m_surface         = context.get_surface();
    m_window          = platform.get_handle();
    m_context         = &context;

    create();
  }

  void Swapchain::create_image_views()
  {
    m_image_views.resize(m_images.size());

    for (size_t i = 0; i < m_images.size(); i++)
    {
      VkImageViewCreateInfo create_info{};
      create_info.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
      create_info.image                           = m_images[i];
      create_info.viewType                        = VK_IMAGE_VIEW_TYPE_2D;
      create_info.format                          = m_image_format;
      create_info.components.r                    = VK_COMPONENT_SWIZZLE_IDENTITY;
      create_info.components.g                    = VK_COMPONENT_SWIZZLE_IDENTITY;
      create_info.components.b                    = VK_COMPONENT_SWIZZLE_IDENTITY;
      create_info.components.a                    = VK_COMPONENT_SWIZZLE_IDENTITY;
      create_info.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
      create_info.subresourceRange.baseMipLevel   = 0;
      create_info.subresourceRange.levelCount     = 1;
      create_info.subresourceRange.baseArrayLayer = 0;
      create_info.subresourceRange.layerCount     = 1;

      if (vkCreateImageView(m_device, &create_info, nullptr, &m_image_views[i]) != VK_SUCCESS)
        LOG_ERR("[Swapchain] failed to create image view");
    }

    LOG_PASS("[Swapchain] image views created ({})", m_image_views.size());
  }

  Swapchain::~Swapchain()
  {
    destroy();
  }
}
