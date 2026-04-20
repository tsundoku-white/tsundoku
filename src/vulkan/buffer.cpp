#include "buffer.h"
#include "vulkan/vk_context.h"
#include <cstring>

namespace tsundoku
{
  // ---- find_memory_type -------------------------------------------------------
  // Walks the physical device's memory types to find one that satisfies both
  // the type filter (which bits Vulkan says are compatible) and the property
  // flags we need (e.g. HOST_VISIBLE | HOST_COHERENT for staging buffers,
  // DEVICE_LOCAL for GPU-only buffers).
  static uint32_t find_memory_type(VkPhysicalDevice      physical_device,
                                   uint32_t              type_filter,
                                   VkMemoryPropertyFlags properties)
  {
    VkPhysicalDeviceMemoryProperties mem_props{};
    vkGetPhysicalDeviceMemoryProperties(physical_device, &mem_props);

    for (uint32_t i = 0; i < mem_props.memoryTypeCount; i++)
    {
      bool type_match = (type_filter & (1u << i)) != 0;
      bool prop_match = (mem_props.memoryTypes[i].propertyFlags & properties) == properties;
      if (type_match && prop_match)
        return i;
    }

    LOG_ERR("[Buffer] failed to find suitable memory type");
    return UINT32_MAX; // unreachable if LOG_ERR aborts, but satisfies the compiler
  }

  // ---- create_buffer ----------------------------------------------------------
  Buffer create_buffer(VkContext&            context,
                       VkDeviceSize          size,
                       VkBufferUsageFlags    usage,
                       VkMemoryPropertyFlags properties)
  {
    VkDevice         device          = context.get_device();
    VkPhysicalDevice physical_device = context.get_physical_device();

    VkBufferCreateInfo buffer_info{};
    buffer_info.sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_info.size        = size;
    buffer_info.usage       = usage;
    buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    Buffer buf{};
    if (vkCreateBuffer(device, &buffer_info, nullptr, &buf.handle) != VK_SUCCESS)
    {
      LOG_ERR("[Buffer] failed to create buffer");
      return buf;
    }

    VkMemoryRequirements mem_reqs{};
    vkGetBufferMemoryRequirements(device, buf.handle, &mem_reqs);

    VkMemoryAllocateInfo alloc_info{};
    alloc_info.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.allocationSize  = mem_reqs.size;
    alloc_info.memoryTypeIndex = find_memory_type(physical_device,
                                                  mem_reqs.memoryTypeBits,
                                                  properties);

    if (vkAllocateMemory(device, &alloc_info, nullptr, &buf.memory) != VK_SUCCESS)
    {
      LOG_ERR("[Buffer] failed to allocate buffer memory");
      vkDestroyBuffer(device, buf.handle, nullptr);
      buf.handle = VK_NULL_HANDLE;
      return buf;
    }

    vkBindBufferMemory(device, buf.handle, buf.memory, 0);
    return buf;
  }

  // ---- copy_buffer ------------------------------------------------------------
  // Allocates a transient command buffer, records a vkCmdCopyBuffer, submits it,
  // and waits for completion. Suitable for one-shot uploads at load time.
  void copy_buffer(VkContext& context,
                   VkBuffer src, VkBuffer dst, VkDeviceSize size)
  {
    VkDevice      device       = context.get_device();
    VkCommandPool command_pool = context.get_command_pool();
    VkQueue       queue        = context.get_graphics_queue();

    VkCommandBufferAllocateInfo alloc_info{};
    alloc_info.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    alloc_info.commandPool        = command_pool;
    alloc_info.commandBufferCount = 1;

    VkCommandBuffer cmd{};
    vkAllocateCommandBuffers(device, &alloc_info, &cmd);

    VkCommandBufferBeginInfo begin_info{};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(cmd, &begin_info);

    VkBufferCopy region{};
    region.srcOffset = 0;
    region.dstOffset = 0;
    region.size      = size;
    vkCmdCopyBuffer(cmd, src, dst, 1, &region);

    vkEndCommandBuffer(cmd);

    VkSubmitInfo submit_info{};
    submit_info.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers    = &cmd;

    vkQueueSubmit(queue, 1, &submit_info, VK_NULL_HANDLE);
    vkQueueWaitIdle(queue); // fine for load-time uploads

    vkFreeCommandBuffers(device, command_pool, 1, &cmd);
  }

  // ---- destroy_buffer ---------------------------------------------------------
  void destroy_buffer(VkDevice device, Buffer& buffer)
  {
    if (buffer.handle != VK_NULL_HANDLE)
    {
      vkDestroyBuffer(device, buffer.handle, nullptr);
      buffer.handle = VK_NULL_HANDLE;
    }
    if (buffer.memory != VK_NULL_HANDLE)
    {
      vkFreeMemory(device, buffer.memory, nullptr);
      buffer.memory = VK_NULL_HANDLE;
    }
  }

  // ---- upload_buffer ----------------------------------------------------------
  // Creates a HOST_VISIBLE staging buffer, copies the CPU data into it, then
  // does a GPU-side copy into a DEVICE_LOCAL buffer and destroys the staging buf.
  Buffer upload_buffer(VkContext&         context,
                       const void*        data,
                       VkDeviceSize       size,
                       VkBufferUsageFlags usage)
  {
    VkDevice device = context.get_device();

    // Staging buffer: CPU-writable
    Buffer staging = create_buffer(context,
                                   size,
                                   VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                   VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    void* mapped = nullptr;
    vkMapMemory(device, staging.memory, 0, size, 0, &mapped);
    memcpy(mapped, data, static_cast<size_t>(size));
    vkUnmapMemory(device, staging.memory);

    // Device-local destination buffer
    Buffer gpu_buffer = create_buffer(context,
                                      size,
                                      VK_BUFFER_USAGE_TRANSFER_DST_BIT | usage,
                                      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    copy_buffer(context, staging.handle, gpu_buffer.handle, size);
    destroy_buffer(device, staging);

    return gpu_buffer;
  }

} // namespace tsundoku
