#include "vulkan/sync.h"
#include "core/common.h"
#include "vulkan/vk_context.h"

namespace tsundoku
{
  Sync::Sync(VkContext &context)
  {
    m_device = context.get_device();

    m_image_available.resize(FRAMES_IN_FLIGHT);
    m_render_finished.resize(FRAMES_IN_FLIGHT);
    m_in_flight.resize(FRAMES_IN_FLIGHT);

    VkSemaphoreCreateInfo semaphore_info{};
    semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fence_info{};
    fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < FRAMES_IN_FLIGHT; i++) {
      if (vkCreateSemaphore(m_device, &semaphore_info, nullptr, &m_image_available[i]) != VK_SUCCESS ||
          vkCreateSemaphore(m_device, &semaphore_info, nullptr, &m_render_finished[i]) != VK_SUCCESS ||
          vkCreateFence(m_device, &fence_info, nullptr, &m_in_flight[i]) != VK_SUCCESS) {

        LOG_ERR("[Sync] failed to create sync objects");
      }
    }
    LOG_PASS("[Sync] created");
  }

  Sync::~Sync()
  {
    for (size_t i = 0; i < FRAMES_IN_FLIGHT; i++) {
      vkDestroySemaphore(m_device, m_render_finished[i], nullptr);
      vkDestroySemaphore(m_device, m_image_available[i], nullptr);
      vkDestroyFence(m_device, m_in_flight[i], nullptr);
    }
  }
}
