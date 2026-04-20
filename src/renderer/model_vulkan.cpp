#include "model_vulkan.h"
#include "core/common.h"
#include "vulkan/swapchain.h"
#include "vulkan/vk_context.h"
#include "vulkan/buffer.h"
#include "components/model.h"
#include <cstring>
#include <vulkan/vulkan_core.h>

namespace tsundoku
{
  // ---- static layout factory --------------------------------------------------
  VkDescriptorSetLayout ModelVulkan::create_layout(VkDevice device)
  {
    VkDescriptorSetLayoutBinding ubo_binding{};
    ubo_binding.binding            = 0;
    ubo_binding.descriptorType     = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    ubo_binding.descriptorCount    = 1;
    ubo_binding.stageFlags         = VK_SHADER_STAGE_VERTEX_BIT;
    ubo_binding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutCreateInfo layout_info{};
    layout_info.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layout_info.bindingCount = 1;
    layout_info.pBindings    = &ubo_binding;

    VkDescriptorSetLayout layout = VK_NULL_HANDLE;
    if (vkCreateDescriptorSetLayout(device, &layout_info, nullptr, &layout) != VK_SUCCESS)
      LOG_ERR("[ModelVulkan] failed to create descriptor set layout");

    return layout;
  }

  ModelVulkan::ModelVulkan(Model& model, VkContext& context, Swapchain& swapchain)
    : m_swapchain(&swapchain), m_model(&model)
  {
    m_device          = context.get_device();
    m_physical_device = context.get_physical_device();

    m_descriptor_layout = create_layout(m_device);

    create_descriptor_pool();

    for (uint32_t i = 0; i < FRAMES_IN_FLIGHT; i++)
    {
      m_frames[i].ubo_buffer = create_buffer(context,
                                             sizeof(UBO),
                                             VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                             VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

      vkMapMemory(m_device, m_frames[i].ubo_buffer.memory, 0, sizeof(UBO), 0, &m_frames[i].mapped);
    }

    upload_mesh(context);
    create_descriptor_sets();
    LOG_PASS("[ModelVulkan] created");
  }

  ModelVulkan::~ModelVulkan()
  {
    for (auto& f : m_frames)
    {
      if (f.mapped)
        vkUnmapMemory(m_device, f.ubo_buffer.memory);
      destroy_buffer(m_device, f.ubo_buffer);
    }

    destroy_buffer(m_device, m_vertex_buffer);
    destroy_buffer(m_device, m_index_buffer);

    vkDestroyDescriptorPool(m_device, m_descriptor_pool, nullptr);
    vkDestroyDescriptorSetLayout(m_device, m_descriptor_layout, nullptr);
  }

  void ModelVulkan::draw(VkCommandBuffer cmd, VkPipelineLayout layout, uint32_t current_frame)
  {
    UBO ubo{};
    ubo.model = m_model->model_matrix;
    memcpy(m_frames[current_frame].mapped, &ubo, sizeof(UBO));

    VkBuffer     vertex_buffers[] = { m_vertex_buffer.handle };
    VkDeviceSize offsets[]        = { 0 };
    vkCmdBindVertexBuffers(cmd, 0, 1, vertex_buffers, offsets);
    vkCmdBindIndexBuffer(cmd, m_index_buffer.handle, 0, VK_INDEX_TYPE_UINT32);

    VkDescriptorSet model_set = m_frames[current_frame].descriptor;
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            layout, 1, 1, &model_set, 0, nullptr);

    vkCmdDrawIndexed(cmd, m_index_count, 1, 0, 0, 0);
  }

  void ModelVulkan::upload_mesh(VkContext& context)
  {
    const Model::Mesh* mesh = m_model->get_mesh();
    if (!mesh)
    {
      LOG_ERR("[ModelVulkan] model has no mesh data to upload");
      return;
    }

    VkDeviceSize vertex_size = sizeof(Model::Vertex) * mesh->vertices.size();
    m_vertex_buffer = upload_buffer(context,
                                    mesh->vertices.data(),
                                    vertex_size,
                                    VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);

    VkDeviceSize index_size = sizeof(uint32_t) * mesh->indices.size();
    m_index_buffer = upload_buffer(context,
                                   mesh->indices.data(),
                                   index_size,
                                   VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

    m_index_count = static_cast<uint32_t>(mesh->indices.size());

    LOG_PASS("[ModelVulkan] mesh uploaded ({} vertices, {} indices)",
             mesh->vertices.size(), mesh->indices.size());
  }

  void ModelVulkan::create_descriptor_pool()
  {
    VkDescriptorPoolSize pool_size{};
    pool_size.type            = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    pool_size.descriptorCount = FRAMES_IN_FLIGHT;

    VkDescriptorPoolCreateInfo pool_info{};
    pool_info.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.poolSizeCount = 1;
    pool_info.pPoolSizes    = &pool_size;
    pool_info.maxSets       = FRAMES_IN_FLIGHT;

    if (vkCreateDescriptorPool(m_device, &pool_info, nullptr, &m_descriptor_pool) != VK_SUCCESS)
      LOG_ERR("[ModelVulkan] failed to create descriptor pool");
  }

  void ModelVulkan::create_descriptor_sets()
  {
    std::array<VkDescriptorSetLayout, FRAMES_IN_FLIGHT> layouts;
    layouts.fill(m_descriptor_layout);

    VkDescriptorSetAllocateInfo alloc_info{};
    alloc_info.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    alloc_info.descriptorPool     = m_descriptor_pool;
    alloc_info.descriptorSetCount = FRAMES_IN_FLIGHT;
    alloc_info.pSetLayouts        = layouts.data();

    std::array<VkDescriptorSet, FRAMES_IN_FLIGHT> sets;
    if (vkAllocateDescriptorSets(m_device, &alloc_info, sets.data()) != VK_SUCCESS)
      LOG_ERR("[ModelVulkan] failed to allocate descriptor sets");

    for (uint32_t i = 0; i < FRAMES_IN_FLIGHT; i++)
    {
      m_frames[i].descriptor = sets[i];

      VkDescriptorBufferInfo buffer_info{};
      buffer_info.buffer = m_frames[i].ubo_buffer.handle;
      buffer_info.offset = 0;
      buffer_info.range  = sizeof(UBO);

      VkWriteDescriptorSet write{};
      write.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
      write.dstSet          = m_frames[i].descriptor;
      write.dstBinding      = 0;
      write.dstArrayElement = 0;
      write.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
      write.descriptorCount = 1;
      write.pBufferInfo     = &buffer_info;

      vkUpdateDescriptorSets(m_device, 1, &write, 0, nullptr);
    }
  }

} // namespace tsundoku
