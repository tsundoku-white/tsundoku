#pragma once

#include "core/common.h"
#include <vulkan/vulkan_core.h>

namespace tsundoku
{
  class VkContext;

  struct Buffer
  {
    VkBuffer       handle = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;

    bool is_valid() const { return handle != VK_NULL_HANDLE; }
  };

  // Allocates a buffer and its backing memory.
  Buffer create_buffer(VkContext& context,
                       VkDeviceSize          size,
                       VkBufferUsageFlags    usage,
                       VkMemoryPropertyFlags properties);

  // Copies src into dst via a transient command buffer on the graphics queue.
  void   copy_buffer(VkContext& context,
                     VkBuffer src, VkBuffer dst, VkDeviceSize size);

  // Destroys the buffer and frees its memory.
  void   destroy_buffer(VkDevice device, Buffer& buffer);

  // Uploads arbitrary CPU data to a DEVICE_LOCAL buffer via a staging buffer.
  Buffer upload_buffer(VkContext& context,
                       const void*        data,
                       VkDeviceSize       size,
                       VkBufferUsageFlags usage);
}
