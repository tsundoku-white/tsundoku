#pragma once

#include "core/common.h"

struct GLFWwindow;

namespace tsundoku
{
  class Platform;

  class VkContext
  {
    public:
      VkContext(Platform &platform);
      ~VkContext();

      VkContext(const VkContext&)              = delete;
      VkContext& operator = (const VkContext&) = delete;

      VkInstance       get_instance()        const { return m_instance       ; }
      VkPhysicalDevice get_physical_device() const { return m_physical_device; }
      VkDevice         get_device()          const { return m_device         ; }
      VkQueue          get_graphics_queue()  const { return m_graphics_queue ; }
      VkQueue          get_present_queue()   const { return m_present_queue  ; }
      VkSurfaceKHR     get_surface()         const { return m_surface        ; }

      struct QueueFamilyIndices
      {
        std::optional<uint32_t> graphics_family;
        std::optional<uint32_t> present_family;

        bool is_complete() const
        {
          return graphics_family.has_value() && present_family.has_value();
        }
      };
      QueueFamilyIndices get_queue_families() const { return m_queue_families; }

      struct SwapchainSupportDetails
      {
        VkSurfaceCapabilitiesKHR        capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR>   present_modes;
      };

    private:
      VkInstance               m_instance         = VK_NULL_HANDLE;
      VkDebugUtilsMessengerEXT m_debug_messenger  = VK_NULL_HANDLE;
      VkPhysicalDevice         m_physical_device  = VK_NULL_HANDLE;
      VkDevice                 m_device           = VK_NULL_HANDLE;
      VkQueue                  m_graphics_queue   = VK_NULL_HANDLE;
      VkQueue                  m_present_queue    = VK_NULL_HANDLE;
      VkSurfaceKHR             m_surface          = VK_NULL_HANDLE;
      QueueFamilyIndices       m_queue_families   = {};

      void create_instance();
      void setup_debug_messenger();
      void create_surface(GLFWwindow *window_handle);
      void pick_physical_device();
      void create_logical_device();
  };
} // namespace tsundoku

