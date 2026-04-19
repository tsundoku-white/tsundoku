#include "vulkan/vertex_buffer.h"
#include "vulkan/vk_context.h"
#include "core/common.h"
#include <cstring>

namespace tsundoku
{
  VertexBuffer::VertexBuffer(VkContext& context)
    : m_device(context.get_device())
    , m_physical(context.get_physical_device())
    , m_graphics_queue(context.get_graphics_queue())
    , m_queue_family(context.get_queue_families().graphics_family.value())
  {
    LOG_PASS("[VertexBuffer] created");
  }

  VertexBuffer::~VertexBuffer()
  {
    if (m_vertex_buffer)        vkDestroyBuffer(m_device, m_vertex_buffer,       nullptr);
    if (m_vertex_buffer_memory) vkFreeMemory(m_device, m_vertex_buffer_memory,   nullptr);
    if (m_index_buffer)         vkDestroyBuffer(m_device, m_index_buffer,        nullptr);
    if (m_index_buffer_memory)  vkFreeMemory(m_device, m_index_buffer_memory,    nullptr);
  }

  void VertexBuffer::upload_vertices(const std::vector<Vertex>& vertices)
  {
    if (vertices.empty()) return;

    VkDeviceSize size = sizeof(vertices[0]) * vertices.size();

    VkBuffer       staging_buffer;
    VkDeviceMemory staging_memory;
    create_buffer(size,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        staging_buffer, staging_memory);

    void* data;
    vkMapMemory(m_device, staging_memory, 0, size, 0, &data);
    memcpy(data, vertices.data(), static_cast<size_t>(size));
    vkUnmapMemory(m_device, staging_memory);

    create_buffer(size,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        m_vertex_buffer, m_vertex_buffer_memory);

    copy_buffer(staging_buffer, m_vertex_buffer, size);

    vkDestroyBuffer(m_device, staging_buffer, nullptr);
    vkFreeMemory(m_device, staging_memory, nullptr);

    LOG_PASS("[VertexBuffer] vertices uploaded");
  }

  void VertexBuffer::upload_indices(const std::vector<uint32_t>& indices)
  {
    if (indices.empty()) return;

    m_index_count = static_cast<uint32_t>(indices.size());
    VkDeviceSize size = sizeof(uint32_t) * indices.size();

    VkBuffer       staging_buffer;
    VkDeviceMemory staging_memory;
    create_buffer(size,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        staging_buffer, staging_memory);

    void* data;
    vkMapMemory(m_device, staging_memory, 0, size, 0, &data);
    memcpy(data, indices.data(), static_cast<size_t>(size));
    vkUnmapMemory(m_device, staging_memory);

    create_buffer(size,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        m_index_buffer, m_index_buffer_memory);

    copy_buffer(staging_buffer, m_index_buffer, size);

    vkDestroyBuffer(m_device, staging_buffer, nullptr);
    vkFreeMemory(m_device, staging_memory, nullptr);

    LOG_PASS("[VertexBuffer] indices uploaded");
  }

  void VertexBuffer::bind_vertex(VkCommandBuffer cmd)
  {
    VkBuffer     buffers[] = { m_vertex_buffer };
    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(cmd, 0, 1, buffers, offsets);
  }

  void VertexBuffer::bind_index(VkCommandBuffer cmd)
  {
    vkCmdBindIndexBuffer(cmd, m_index_buffer, 0, VK_INDEX_TYPE_UINT32);
  }

  void VertexBuffer::create_buffer(VkDeviceSize size, VkBufferUsageFlags usage,
                                   VkMemoryPropertyFlags props,
                                   VkBuffer& buffer, VkDeviceMemory& memory)
  {
    VkBufferCreateInfo buffer_info{};
    buffer_info.sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_info.size        = size;
    buffer_info.usage       = usage;
    buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(m_device, &buffer_info, nullptr, &buffer) != VK_SUCCESS)
      LOG_ERR("[VertexBuffer] failed to create buffer");

    VkMemoryRequirements mem_reqs;
    vkGetBufferMemoryRequirements(m_device, buffer, &mem_reqs);

    VkMemoryAllocateInfo alloc_info{};
    alloc_info.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.allocationSize  = mem_reqs.size;
    alloc_info.memoryTypeIndex = find_memory_type(mem_reqs.memoryTypeBits, props);

    if (vkAllocateMemory(m_device, &alloc_info, nullptr, &memory) != VK_SUCCESS)
      LOG_ERR("[VertexBuffer] failed to allocate memory");

    vkBindBufferMemory(m_device, buffer, memory, 0);
  }

  void VertexBuffer::copy_buffer(VkBuffer src, VkBuffer dst, VkDeviceSize size)
  {
    VkCommandPoolCreateInfo pool_info{};
    pool_info.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pool_info.flags            = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    pool_info.queueFamilyIndex = m_queue_family;

    VkCommandPool pool;
    vkCreateCommandPool(m_device, &pool_info, nullptr, &pool);

    VkCommandBufferAllocateInfo alloc_info{};
    alloc_info.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    alloc_info.commandPool        = pool;
    alloc_info.commandBufferCount = 1;

    VkCommandBuffer cmd;
    vkAllocateCommandBuffers(m_device, &alloc_info, &cmd);

    VkCommandBufferBeginInfo begin_info{};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(cmd, &begin_info);

    VkBufferCopy copy_region{};
    copy_region.size = size;
    vkCmdCopyBuffer(cmd, src, dst, 1, &copy_region);

    vkEndCommandBuffer(cmd);

    VkSubmitInfo submit_info{};
    submit_info.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers    = &cmd;

    vkQueueSubmit(m_graphics_queue, 1, &submit_info, VK_NULL_HANDLE);
    vkQueueWaitIdle(m_graphics_queue);

    vkFreeCommandBuffers(m_device, pool, 1, &cmd);
    vkDestroyCommandPool(m_device, pool, nullptr);
  }

  uint32_t VertexBuffer::find_memory_type(uint32_t type_filter, VkMemoryPropertyFlags props)
  {
    VkPhysicalDeviceMemoryProperties mem_props;
    vkGetPhysicalDeviceMemoryProperties(m_physical, &mem_props);
    for (uint32_t i = 0; i < mem_props.memoryTypeCount; i++) {
      if ((type_filter & (1 << i)) &&
          (mem_props.memoryTypes[i].propertyFlags & props) == props)
        return i;
    }
    LOG_ERR("[VertexBuffer] failed to find suitable memory type");
    return 0;
  }
}
