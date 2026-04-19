#pragma once

#include "core/common.h"
#include <vulkan/vulkan_core.h>

struct GLFWwindow;

namespace tsundoku
{
  class Platform;
  class VkContext;

  class Swapchain 
  {
    public:
      Swapchain(Platform &platform, VkContext &context);
      ~Swapchain();

      Swapchain(const Swapchain&) = delete;
      Swapchain& operator = (const Swapchain&) = delete;

      VkFormat get_format() const { return m_image_format; };
      std::vector<VkImageView> get_image_views() {return m_image_views; }
      VkExtent2D get_extent() {return m_extent; }
      VkSwapchainKHR get_swapchain() { return m_swapchain; }

      void create();
      void destroy();

    private:
      VkSwapchainKHR           m_swapchain     = VK_NULL_HANDLE;
      VkSwapchainKHR           m_old_swapchain     = VK_NULL_HANDLE;
      VkFormat                 m_image_format  = {};
      VkExtent2D               m_extent        = {};
      GLFWwindow*              m_window        = nullptr;

      std::vector<VkImage>     m_images;
      std::vector<VkImageView> m_image_views;

      VkDevice         m_device          = VK_NULL_HANDLE;
      VkPhysicalDevice m_physical_device = VK_NULL_HANDLE;
      VkSurfaceKHR     m_surface         = VK_NULL_HANDLE;
      VkContext*       m_context         = nullptr;

      struct SwapChainSupportDetails 
      {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
      };

      void create_image_views();
      VkSurfaceFormatKHR choose_surface_format(const std::vector<VkSurfaceFormatKHR>& formats);
      VkPresentModeKHR   choose_present_mode  (const std::vector<VkPresentModeKHR>&   modes);
      VkExtent2D         choose_extent        (const VkSurfaceCapabilitiesKHR&         caps);
  };
}
