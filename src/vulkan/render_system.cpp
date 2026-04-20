#include "render_system.h"

#include "core/platform.h"
#include "core/scene.h"
#include "core/entity.h"
#include "vulkan/vk_context.h"
#include "vulkan/swapchain.h"
#include "vulkan/render_pass.h"
#include "vulkan/pipeline.h"
#include "vulkan/framebuffer.h"
#include "vulkan/sync.h"
#include "vulkan/frame_cap.h"
#include "renderer/camera_vulkan.h"
#include "renderer/model_vulkan.h"
#include "components/camera.h"
#include "components/model.h"

namespace tsundoku
{
  RenderSystem::RenderSystem(Platform &platform, Camera &camera)
  {
    m_platform      = &platform;
    m_context       = std::make_unique<VkContext>(platform);
    m_swapchain     = std::make_unique<Swapchain>(platform, *m_context);
    m_render_pass   = std::make_unique<RenderPass>(*m_context, *m_swapchain);
    m_framebuffer   = std::make_unique<Framebuffer>(*m_context, *m_swapchain, *m_render_pass);
    m_camera_vulkan = std::make_unique<CameraVulkan>(camera, *m_context, platform, *m_swapchain);
    m_pipeline      = std::make_unique<Pipeline>(*m_context, *m_swapchain, *m_render_pass, *m_camera_vulkan);
    m_sync          = std::make_unique<Sync>(*m_context);

    m_device         = m_context->get_device();
    m_graphics_queue = m_context->get_graphics_queue();
    m_present_queue  = m_context->get_present_queue();
  }

  void RenderSystem::frame_pass(Scene &scene)
  {
    // Update first so matrices are ready before draw
    float dt = m_frame_cap.delta_time;
    for (auto& [id, entity] : scene.get_entities())
    {
      for (auto& [type, component] : entity->get_components())
        component->update(dt);
      entity->transform.update(dt);
    }

    // Then lazy upload and draw
    for (auto& [id, entity] : scene.get_entities())
    {
      if (m_model_vulkans.count(id) == 0 && entity->has_component<Model>())
      {
        Model* model = entity->get_component<Model>();
        if (model->get_mesh())
          m_model_vulkans[id] = std::make_unique<ModelVulkan>(*model, *m_context, *m_swapchain);
      }
    }

    draw_frame(scene);
  }

  void RenderSystem::draw_frame(Scene &scene)
  {
    m_frame_cap.start();
    VkFence     fence     = m_sync->get_in_flight(m_current_frame);
    VkSemaphore img_avail = m_sync->get_image_available(m_current_frame);
    VkSemaphore rnd_done  = m_sync->get_render_finished(m_current_frame);

    vkWaitForFences(m_device, 1, &fence, VK_TRUE, UINT64_MAX);

    if (m_platform->has_resized())
    {
      recreate_swapchain();
      m_platform->set_resized(false);
      return;
    }

    uint32_t image_index = 0;
    VkResult result = vkAcquireNextImageKHR(m_device, m_swapchain->get_swapchain(),
        UINT64_MAX, img_avail, VK_NULL_HANDLE, &image_index);

    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
      recreate_swapchain();
      return;
    }

    vkResetFences(m_device, 1, &fence);

    m_context->reset(m_current_frame);
    m_camera_vulkan->draw(m_current_frame);
    record(scene, image_index);

    VkSemaphore          wait_semaphores[]   = { img_avail };
    VkPipelineStageFlags wait_stages[]       = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    VkSemaphore          signal_semaphores[] = { rnd_done  };
    VkCommandBuffer      cmd                 = m_context->get_buffers(m_current_frame);

    VkSubmitInfo submit_info{};
    submit_info.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.waitSemaphoreCount   = 1;
    submit_info.pWaitSemaphores      = wait_semaphores;
    submit_info.pWaitDstStageMask    = wait_stages;
    submit_info.commandBufferCount   = 1;
    submit_info.pCommandBuffers      = &cmd;
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores    = signal_semaphores;

    if (vkQueueSubmit(m_graphics_queue, 1, &submit_info, fence) != VK_SUCCESS)
      LOG_ERR("[RenderSystem] failed to submit draw command buffer");

    VkSwapchainKHR swapchains[] = { m_swapchain->get_swapchain() };

    VkPresentInfoKHR present_info{};
    present_info.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores    = signal_semaphores;
    present_info.swapchainCount     = 1;
    present_info.pSwapchains        = swapchains;
    present_info.pImageIndices      = &image_index;

    result = vkQueuePresentKHR(m_present_queue, &present_info);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
      recreate_swapchain();

    m_current_frame = (m_current_frame + 1) % FRAMES_IN_FLIGHT;
    m_frame_cap.end();
    delta_time = m_frame_cap.delta_time;
  }

  void RenderSystem::record(Scene &scene, uint32_t image_index)
  {
    m_context->begin(m_current_frame);

    // Prepare clear values for BOTH attachments
    std::array<VkClearValue, 2> clearValues{};

    // Color attachment (index 0) - clear to black
    clearValues[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};

    // Depth attachment (index 1) - clear to far plane
    clearValues[1].depthStencil = {1.0f, 0};

    VkRenderPassBeginInfo render_pass_info{};
    render_pass_info.sType             = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_info.renderPass        = m_render_pass->get_render_pass();
    render_pass_info.framebuffer       = m_framebuffer->get_framebuffers()[image_index];
    render_pass_info.renderArea.offset = {0, 0};
    render_pass_info.renderArea.extent = m_swapchain->get_extent();
    render_pass_info.clearValueCount = static_cast<uint32_t>(clearValues.size());
    render_pass_info.pClearValues      = clearValues.data();

    VkCommandBuffer cmd    = m_context->get_buffers(m_current_frame);
    VkExtent2D      extent = m_swapchain->get_extent();

    vkCmdBeginRenderPass(cmd, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline->get_pipeline());

    VkViewport viewport{};
    viewport.x        = 0.0f;
    viewport.y        = 0.0f;
    viewport.width    = static_cast<float>(extent.width);
    viewport.height   = static_cast<float>(extent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(cmd, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = extent;
    vkCmdSetScissor(cmd, 0, 1, &scissor);

    // Camera bound once at set=0 for all models
    VkDescriptorSet camera_set = m_camera_vulkan->get_descriptor_set(m_current_frame);
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
        m_pipeline->get_layout(), 0, 1, &camera_set, 0, nullptr);

    // Each model binds its own vertex/index buffers and set=1, then draws
    for (auto& [id, model_vulkan] : m_model_vulkans)
      model_vulkan->draw(cmd, m_pipeline->get_layout(), m_current_frame);

    vkCmdEndRenderPass(cmd);
    m_context->end(m_current_frame);
  }

  void RenderSystem::recreate_swapchain()
  {
    vkDeviceWaitIdle(m_device);
    m_framebuffer->destroy();
    m_swapchain->destroy();
    m_swapchain->create();
    m_framebuffer->create(*m_swapchain);
  }

  RenderSystem::~RenderSystem()
  {
    vkDeviceWaitIdle(m_device);
  }
}
