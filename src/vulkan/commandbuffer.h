#pragma once

#include "core/common.h"
#include <vector>

namespace tsundoku
{
  class VkContext;

  class CommandBuffer
  {
    public:
      CommandBuffer(VkContext &context);
      ~CommandBuffer();

      CommandBuffer(const CommandBuffer&) = delete;
      CommandBuffer& operator=(const CommandBuffer&) = delete;

      void begin(uint32_t frame);
      void end(uint32_t frame);
      void reset(uint32_t frame);

      VkCommandBuffer get(uint32_t frame) const { return m_command_buffers[frame]; }

    private:
      VkDevice                     m_device          = VK_NULL_HANDLE;
      VkCommandPool                m_command_pool    = VK_NULL_HANDLE;
      std::vector<VkCommandBuffer> m_command_buffers = {};
  };
}
