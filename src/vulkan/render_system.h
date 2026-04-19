#pragma once 

#include "core/common.h"
#include "vulkan/frame_cap.h"
#include <memory>

namespace tsundoku
{
  class Platform;
  class VkContext;
  class Swapchain;
  class RenderPass;
  class Pipeline;
  class Framebuffer;
  class VertexBuffer;
  class CommandBuffer;
  class Sync;
  class CameraVulkan;
  class Camera;
  class Entity;
  class Scene;

  class RenderSystem
  {
    public:
      RenderSystem(Platform &platform, Camera &camera);
      void frame_pass(Scene &scene);
      ~RenderSystem();

      float delta_time = 0.f;

    private:
      Platform                      *m_platform;
      std::unique_ptr<VkContext>     m_context;
      std::unique_ptr<Swapchain>     m_swapchain;
      std::unique_ptr<RenderPass>    m_render_pass;
      std::unique_ptr<Framebuffer>   m_framebuffer;
      std::unique_ptr<VertexBuffer>  m_vertex_buffer;
      std::unique_ptr<CameraVulkan>  m_camera_vulkan;
      std::unique_ptr<Pipeline>      m_pipeline;
      std::unique_ptr<CommandBuffer> m_command_buffer;
      std::unique_ptr<Sync>          m_sync;

      VkDevice m_device         = VK_NULL_HANDLE;
      VkQueue  m_graphics_queue = VK_NULL_HANDLE;
      VkQueue  m_present_queue  = VK_NULL_HANDLE;

      FrameCap m_frame_cap;
      uint32_t m_current_frame = 0;

      void draw_frame();
      void record(uint32_t image_index);
      void recreate_swapchain();
  };
}
