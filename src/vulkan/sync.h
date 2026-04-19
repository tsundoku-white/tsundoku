#pragma once

#include "core/common.h"

namespace tsundoku
{
  class VkContext;

  class Sync
  {
    public:
      Sync(VkContext &context);
      ~Sync();

      Sync(const Sync&) = delete;
      Sync& operator=(const Sync&) = delete;

      VkSemaphore get_image_available(uint32_t frame) const { return m_image_available[frame]; }
      VkSemaphore get_render_finished(uint32_t frame) const { return m_render_finished[frame]; }
      VkFence     get_in_flight      (uint32_t frame) const { return m_in_flight[frame];       }

    private:
      VkDevice                 m_device          = VK_NULL_HANDLE;
      std::vector<VkSemaphore> m_image_available = {};
      std::vector<VkSemaphore> m_render_finished = {};
      std::vector<VkFence>     m_in_flight       = {};
  };
}
