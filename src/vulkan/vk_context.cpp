#include "vk_context.h"
#include "core/common.h"
#include "core/platform.h"

#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <cstring>

namespace tsundoku
{

  namespace {
#ifdef NDEBUG
    constexpr bool ENABLE_VALIDATION_LAYERS = false;
#else
    constexpr bool ENABLE_VALIDATION_LAYERS = true;
#endif

    constexpr const char* VALIDATION_LAYERS[] = {
      "VK_LAYER_KHRONOS_validation",
    };

    constexpr const char* DEVICE_EXTENSIONS[] = {
      VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    };

    bool check_validation_layer_support()
    {
      uint32_t layer_count = 0;
      vkEnumerateInstanceLayerProperties(&layer_count, nullptr);

      std::vector<VkLayerProperties> available_layers(layer_count);
      vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());

      for (const char* layer_name : VALIDATION_LAYERS) {
        bool found = false;
        for (const auto& props : available_layers) {
          if (strcmp(layer_name, props.layerName) == 0) {
            found = true;
            break;
          }
        }
        if (!found)
          return false;
      }

      return true;
    }

    std::vector<const char*> get_required_extensions()
    {
      uint32_t     glfw_count      = 0;
      const char** glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_count);

      std::vector<const char*> extensions(glfw_extensions, glfw_extensions + glfw_count);

      if (ENABLE_VALIDATION_LAYERS)
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

      return extensions;
    }

    // Vulkan requires the debug callback to match this exact signature; it cannot be a member.
    VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
        VkDebugUtilsMessageSeverityFlagBitsEXT      message_severity,
        VkDebugUtilsMessageTypeFlagsEXT             message_type,
        const VkDebugUtilsMessengerCallbackDataEXT* p_callback_data,
        void*                                       p_user_data)
    {
      (void)message_severity;
      (void)message_type;
      (void)p_user_data;

      std::string_view msg = p_callback_data->pMessage;
      if (msg.contains("Copying old device"))
        return VK_FALSE;

      LOG_WARN("[Debug Messenger] validation layer: {}\n", p_callback_data->pMessage);
      return VK_FALSE;
    }

    // vkCreateDebugUtilsMessengerEXT is not statically linked; load it at runtime.
    VkResult create_debug_utils_messenger(
        VkInstance                                instance,
        const VkDebugUtilsMessengerCreateInfoEXT* create_info,
        const VkAllocationCallbacks*              allocator,
        VkDebugUtilsMessengerEXT*                 debug_messenger)
    {
      auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
          vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));

      if (func)
        return func(instance, create_info, allocator, debug_messenger);

      return VK_ERROR_EXTENSION_NOT_PRESENT;
    }

    void destroy_debug_utils_messenger(
        VkInstance                   instance,
        VkDebugUtilsMessengerEXT     debug_messenger,
        const VkAllocationCallbacks* allocator)
    {
      auto func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
          vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));

      if (func)
        func(instance, debug_messenger, allocator);
    }

    VkContext::QueueFamilyIndices find_queue_families(VkPhysicalDevice device, VkSurfaceKHR surface)
    {
      VkContext::QueueFamilyIndices indices;

      uint32_t count = 0;
      vkGetPhysicalDeviceQueueFamilyProperties(device, &count, nullptr);

      std::vector<VkQueueFamilyProperties> families(count);
      vkGetPhysicalDeviceQueueFamilyProperties(device, &count, families.data());

      for (uint32_t i = 0; i < count; i++) {
        if (families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
          indices.graphics_family = i;

        VkBool32 present_support = VK_FALSE;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &present_support);
        if (present_support)
          indices.present_family = i;

        if (indices.is_complete())
          break;
      }
      return indices;
    }

    bool is_device_suitable(VkPhysicalDevice device, VkSurfaceKHR surface)
    {
      return find_queue_families(device, surface).is_complete();
    }

  } // namespace (anonymous)

  void VkContext::create_instance()
  {
    // checking if the vulkan validation layer are supported
    if (ENABLE_VALIDATION_LAYERS && !check_validation_layer_support())
      LOG_ERR("[Instance] validation layers requested, but not available");

    // setting up application varables
    VkApplicationInfo app_info{};
    app_info.pApplicationName   = "undefined";
    app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.pEngineName        = "No Engine";
    app_info.engineVersion      = VK_MAKE_VERSION(1, 0, 0);
    app_info.apiVersion         = VK_API_VERSION_1_4;

    // storing required extentions
    auto extensions = get_required_extensions();

    VkInstanceCreateInfo create_info{};
    create_info.sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.pApplicationInfo        = &app_info;
    create_info.enabledExtensionCount   = static_cast<uint32_t>(extensions.size());
    create_info.ppEnabledExtensionNames = extensions.data();

    // checking validation layers is true
    if (ENABLE_VALIDATION_LAYERS) {
      create_info.enabledLayerCount   = static_cast<uint32_t>(std::size(VALIDATION_LAYERS));
      create_info.ppEnabledLayerNames = VALIDATION_LAYERS;
    } else {
      create_info.enabledLayerCount = 0;
    }

    // if all of this is ok then the instance gets created
    if (vkCreateInstance(&create_info, nullptr, &m_instance) != VK_SUCCESS)
      LOG_ERR("[Instance] unable to create instance");

    LOG_PASS("[Instance] created");
  }

  void VkContext::setup_debug_messenger()
  {
    VkDebugUtilsMessengerCreateInfoEXT create_info{};
    create_info.sType           = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
      | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
      | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    create_info.messageType     = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
      | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
      | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    create_info.pfnUserCallback = debug_callback;
    create_info.pUserData       = nullptr;

    if (create_debug_utils_messenger(m_instance, &create_info, nullptr, &m_debug_messenger) != VK_SUCCESS)
      LOG_ERR("[Debug Messenger] unable to create the debug messanger");

    LOG_PASS("[Debug Messenger] created");
  }

  void VkContext::create_surface(GLFWwindow* window_handle)
  {
    if (!window_handle)
      LOG_ERR("[Surface] window handle is null");

    if (glfwCreateWindowSurface(m_instance, window_handle, nullptr, &m_surface) != VK_SUCCESS)
      LOG_ERR("[Surface] unable to create window surface");

    LOG_PASS("[Surface] created");
  }

  void VkContext::pick_physical_device()
  {
    uint32_t device_count = 0;
    vkEnumeratePhysicalDevices(m_instance, &device_count, nullptr);

    if (device_count == 0) {
      LOG_ERR("[Physical Device] no GPUs with Vulkan support");
      return;
    }

    std::vector<VkPhysicalDevice> devices(device_count);
    vkEnumeratePhysicalDevices(m_instance, &device_count, devices.data());

    for (const auto& device : devices) {
      if (is_device_suitable(device, m_surface)) {
        m_physical_device = device;
        break;
      }
    }

    if (m_physical_device == VK_NULL_HANDLE) {
      LOG_ERR("[Physical Device] no suitable GPU found");
      return;
    }

    VkPhysicalDeviceProperties device_props;
    vkGetPhysicalDeviceProperties(m_physical_device, &device_props);
    LOG_PASS("[Physical Device] selected: {}", device_props.deviceName);
  }

  void VkContext::create_logical_device()
  {
    m_queue_families           = find_queue_families(m_physical_device, m_surface);
    QueueFamilyIndices indices = m_queue_families;

    float queue_priority = 1.0f;

    std::set<uint32_t> m_unique_families = {
      indices.graphics_family.value(),
      indices.present_family.value(),
    };

    std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
    for (uint32_t family : m_unique_families) {
      VkDeviceQueueCreateInfo queue_create_info{};
      queue_create_info.sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
      queue_create_info.queueFamilyIndex = family;
      queue_create_info.queueCount       = 1;
      queue_create_info.pQueuePriorities = &queue_priority;
      queue_create_infos.push_back(queue_create_info);
    }

    VkPhysicalDeviceFeatures device_features{};

    VkDeviceCreateInfo create_info{};
    create_info.sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    create_info.pQueueCreateInfos       = queue_create_infos.data();
    create_info.queueCreateInfoCount    = static_cast<uint32_t>(queue_create_infos.size());
    create_info.pEnabledFeatures        = &device_features;
    create_info.enabledExtensionCount   = static_cast<uint32_t>(std::size(DEVICE_EXTENSIONS));
    create_info.ppEnabledExtensionNames = DEVICE_EXTENSIONS;

    if (ENABLE_VALIDATION_LAYERS) {
      create_info.enabledLayerCount   = static_cast<uint32_t>(std::size(VALIDATION_LAYERS));
      create_info.ppEnabledLayerNames = VALIDATION_LAYERS;
    } else {
      create_info.enabledLayerCount = 0;
    }

    if (vkCreateDevice(m_physical_device, &create_info, nullptr, &m_device) != VK_SUCCESS) {
      LOG_ERR("[Device] not able to be created");
      return;
    }

    vkGetDeviceQueue(m_device, indices.graphics_family.value(), 0, &m_graphics_queue);
    vkGetDeviceQueue(m_device, indices.present_family.value(),  0, &m_present_queue);
    LOG_PASS("[Device] created");
  }

  void VkContext::create_command_buffers()
  {

    auto families = get_queue_families();

    VkCommandPoolCreateInfo pool_info{};
    pool_info.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pool_info.flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    pool_info.queueFamilyIndex = families.graphics_family.value();

    if (vkCreateCommandPool(m_device, &pool_info, nullptr, &m_command_pool) != VK_SUCCESS)
      LOG_ERR("[CommandBuffer] failed to create command pool");

    LOG_PASS("[CommandBuffer] command pool created");

    m_command_buffers.resize(FRAMES_IN_FLIGHT);

    VkCommandBufferAllocateInfo alloc_info{};
    alloc_info.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.commandPool        = m_command_pool;
    alloc_info.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    alloc_info.commandBufferCount = FRAMES_IN_FLIGHT;


    if (vkAllocateCommandBuffers(m_device, &alloc_info, m_command_buffers.data()) != VK_SUCCESS)
      LOG_ERR("[CommandBuffer] failed to allocate command buffer");

    LOG_PASS("[CommandBuffer] command buffer allocated");
  }

    void VkContext::begin(uint32_t frame)
  {
    VkCommandBufferBeginInfo begin_info{};
    begin_info.sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags            = 0;
    begin_info.pInheritanceInfo = nullptr;

    if (vkBeginCommandBuffer(m_command_buffers[frame], &begin_info) != VK_SUCCESS)
      LOG_ERR("[CommandBuffer] failed to begin recording");
  }

  void VkContext::end(uint32_t frame)
  {
    if (vkEndCommandBuffer(m_command_buffers[frame]) != VK_SUCCESS)
      LOG_ERR("[CommandBuffer] failed to end recording");
  }

  void VkContext::reset(uint32_t frame)
  {
    vkResetCommandBuffer(m_command_buffers[frame], 0);
  }

  VkContext::VkContext(Platform &platform)
  {
    create_instance();
    if (ENABLE_VALIDATION_LAYERS)
      setup_debug_messenger();
    create_surface(platform.get_handle());
    pick_physical_device();
    create_logical_device();
    create_command_buffers();
  }

  VkContext::~VkContext()
  {
    if (m_command_pool != VK_NULL_HANDLE)
      vkDestroyCommandPool(m_device, m_command_pool, nullptr);
    vkDestroyDevice(m_device, nullptr);
    if (ENABLE_VALIDATION_LAYERS)
      destroy_debug_utils_messenger(m_instance, m_debug_messenger, nullptr);
    vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
    vkDestroyInstance(m_instance, nullptr);
  }

} // namespace tsundoku
