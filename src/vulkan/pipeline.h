#pragma once 

#include "core/common.h"

namespace tsundoku
{
  class VkContext;
  class Swapchain;
  class RenderPass;
  class CameraVulkan;

  class Pipeline 
  {
    public:
      Pipeline(VkContext &context, Swapchain &swapchain, RenderPass &render_pass, CameraVulkan &camera_vulkan);
      ~Pipeline();

      Pipeline(const Pipeline&) = delete;
      Pipeline& operator = (const Pipeline&) = delete;

      VkPipeline get_pipeline() const { return m_graphics_pipeline; }
      VkPipelineLayout get_layout() const { return m_pipeline_layout; }
    private:
      VkDevice         m_device            = VK_NULL_HANDLE;
      VkPipeline       m_graphics_pipeline = VK_NULL_HANDLE;
      VkPipelineLayout m_pipeline_layout   = VK_NULL_HANDLE;

      void create_pipeline();
      static std::vector<char> read_file(std::string path);
      VkShaderModule create_shader_module(const std::vector<char>& code);
  };
}
