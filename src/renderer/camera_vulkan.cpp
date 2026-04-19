#include "camera_vulkan.h"
#include "core/common.h"
#include "components/camera.h"
#include "vulkan/swapchain.h"
#include "vulkan/vk_context.h"
#include <cstring>
#include <vulkan/vulkan_core.h>

namespace tsundoku
{
  CameraVulkan::CameraVulkan(Camera &camera, VkContext &context, Platform &platform,
      Swapchain &swapchain) : m_camera(&camera), m_swapchain(&swapchain)
  {
    m_device          = context.get_device();
    m_physical_device = context.get_physical_device();

    create_descriptor_layout();
    create_descriptor_pool();
    create_buffers();
    create_descriptor_sets();

    LOG_PASS("[CameraVulkan] created");
  }

  CameraVulkan::~CameraVulkan()
  {
    for (auto &f : m_frames)
    {
      vkUnmapMemory(m_device, f.memory);
      vkDestroyBuffer(m_device, f.buffer, nullptr);
      vkFreeMemory(m_device, f.memory, nullptr);
    }
    vkDestroyDescriptorPool(m_device, m_descriptor_pool, nullptr);
    vkDestroyDescriptorSetLayout(m_device, m_descriptor_layout, nullptr);
  }

  void CameraVulkan::draw(uint32_t current_frame)
  {
    update_projection();

    UBO ubo{};
    ubo.view = m_camera->view_matrix;
    ubo.proj = m_camera->projection_matrix;

    memcpy(m_frames[current_frame].mapped, &ubo, sizeof(UBO));
  }

  void CameraVulkan::update_projection()
  {
    float aspect = m_swapchain->get_extent().width / (float)m_swapchain->get_extent().height;

    m_camera->projection_matrix = glm::perspective(
        glm::radians(m_camera->field_of_view),
        aspect,
        m_camera->near_clipping_plane,
        m_camera->far_clipping_plane
    );
    m_camera->projection_matrix[1][1] *= -1;
  }

  void CameraVulkan::create_descriptor_layout()
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

    if (vkCreateDescriptorSetLayout(m_device, &layout_info, nullptr, &m_descriptor_layout) != VK_SUCCESS)
      LOG_ERR("[CameraVulkan] failed to create descriptor set layout");
  }

  void CameraVulkan::create_descriptor_pool()
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
      LOG_ERR("[CameraVulkan] failed to create descriptor pool");
  }

  void CameraVulkan::create_buffers()
  {
    VkDeviceSize buffer_size = sizeof(UBO);

    for (auto &f : m_frames)
    {
      // create buffer
      VkBufferCreateInfo buffer_info{};
      buffer_info.sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
      buffer_info.size        = buffer_size;
      buffer_info.usage       = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
      buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

      if (vkCreateBuffer(m_device, &buffer_info, nullptr, &f.buffer) != VK_SUCCESS)
        LOG_ERR("[CameraVulkan] failed to create UBO buffer");

      // allocate memory
      VkMemoryRequirements mem_reqs;
      vkGetBufferMemoryRequirements(m_device, f.buffer, &mem_reqs);

      VkPhysicalDeviceMemoryProperties mem_props;
      vkGetPhysicalDeviceMemoryProperties(m_physical_device, &mem_props);

      uint32_t memory_type = 0;
      VkMemoryPropertyFlags required = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
      for (uint32_t i = 0; i < mem_props.memoryTypeCount; i++)
      {
        if ((mem_reqs.memoryTypeBits & (1 << i)) &&
            (mem_props.memoryTypes[i].propertyFlags & required) == required)
        {
          memory_type = i;
          break;
        }
      }

      VkMemoryAllocateInfo alloc_info{};
      alloc_info.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
      alloc_info.allocationSize  = mem_reqs.size;
      alloc_info.memoryTypeIndex = memory_type;

      if (vkAllocateMemory(m_device, &alloc_info, nullptr, &f.memory) != VK_SUCCESS)
        LOG_ERR("[CameraVulkan] failed to allocate UBO memory");

      vkBindBufferMemory(m_device, f.buffer, f.memory, 0);
      vkMapMemory(m_device, f.memory, 0, buffer_size, 0, &f.mapped);
    }
  }

  void CameraVulkan::create_descriptor_sets()
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
      LOG_ERR("[CameraVulkan] failed to allocate descriptor sets");

    for (uint32_t i = 0; i < FRAMES_IN_FLIGHT; i++)
    {
      m_frames[i].descriptor = sets[i];

      VkDescriptorBufferInfo buffer_info{};
      buffer_info.buffer = m_frames[i].buffer;
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
}
