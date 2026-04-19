#include "vulkan/commandbuffer.h"
#include "core/common.h"
#include "vulkan/vk_context.h"

namespace tsundoku
{
  CommandBuffer::CommandBuffer(VkContext &context)
  {
    m_device = context.get_device();

    auto families = context.get_queue_families();

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

  CommandBuffer::~CommandBuffer()
  {
    if (m_command_pool != VK_NULL_HANDLE)
      vkDestroyCommandPool(m_device, m_command_pool, nullptr);
  }
  void CommandBuffer::begin(uint32_t frame)
  {
    VkCommandBufferBeginInfo begin_info{};
    begin_info.sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags            = 0;
    begin_info.pInheritanceInfo = nullptr;

    if (vkBeginCommandBuffer(m_command_buffers[frame], &begin_info) != VK_SUCCESS)
      LOG_ERR("[CommandBuffer] failed to begin recording");
  }

  void CommandBuffer::end(uint32_t frame)
  {
    if (vkEndCommandBuffer(m_command_buffers[frame]) != VK_SUCCESS)
      LOG_ERR("[CommandBuffer] failed to end recording");
  }

  void CommandBuffer::reset(uint32_t frame)
  {
    vkResetCommandBuffer(m_command_buffers[frame], 0);
  }
}
