#include "descriptor_set.h"
#include "core/common.h"

namespace tsundoku
{
  DescriptorSet::DescriptorSet(VkDevice device,
      const std::vector<DescriptorBinding>& bindings,
      uint32_t set_count)
    : m_device(device)
  {
    create_layout(bindings);
    create_pool(bindings, set_count);
    allocate_sets(set_count);
  }

  DescriptorSet::~DescriptorSet()
  {
    if (m_pool   != VK_NULL_HANDLE) vkDestroyDescriptorPool(m_device, m_pool, nullptr);
    if (m_layout != VK_NULL_HANDLE) vkDestroyDescriptorSetLayout(m_device, m_layout, nullptr);
  }

  DescriptorSet::DescriptorSet(DescriptorSet&& other) noexcept
    : m_device(other.m_device)
    , m_layout(other.m_layout)
    , m_pool  (other.m_pool)
    , m_sets  (std::move(other.m_sets))
  {
    other.m_device = VK_NULL_HANDLE;
    other.m_layout = VK_NULL_HANDLE;
    other.m_pool   = VK_NULL_HANDLE;
  }

  DescriptorSet& DescriptorSet::operator=(DescriptorSet&& other) noexcept
  {
    if (this == &other) return *this;

    if (m_pool   != VK_NULL_HANDLE) vkDestroyDescriptorPool(m_device, m_pool, nullptr);
    if (m_layout != VK_NULL_HANDLE) vkDestroyDescriptorSetLayout(m_device, m_layout, nullptr);

    m_device = other.m_device;
    m_layout = other.m_layout;
    m_pool   = other.m_pool;
    m_sets   = std::move(other.m_sets);

    other.m_device = VK_NULL_HANDLE;
    other.m_layout = VK_NULL_HANDLE;
    other.m_pool   = VK_NULL_HANDLE;

    return *this;
  }

  void DescriptorSet::create_layout(const std::vector<DescriptorBinding>& bindings)
  {
    std::vector<VkDescriptorSetLayoutBinding> vk_bindings;
    vk_bindings.reserve(bindings.size());

    for (const auto& b : bindings)
    {
      VkDescriptorSetLayoutBinding vkb{};
      vkb.binding            = b.binding;
      vkb.descriptorType     = b.type;
      vkb.descriptorCount    = b.count;
      vkb.stageFlags         = b.stage;
      vkb.pImmutableSamplers = nullptr;
      vk_bindings.push_back(vkb);
    }

    VkDescriptorSetLayoutCreateInfo info{};
    info.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    info.bindingCount = static_cast<uint32_t>(vk_bindings.size());
    info.pBindings    = vk_bindings.data();

    if (vkCreateDescriptorSetLayout(m_device, &info, nullptr, &m_layout) != VK_SUCCESS)
      LOG_ERR("[DescriptorSet] failed to create layout");

    LOG_INFO("[DescriptorSet] layout created");
  }

  void DescriptorSet::create_pool(const std::vector<DescriptorBinding>& bindings, uint32_t set_count)
  {
    std::vector<VkDescriptorPoolSize> pool_sizes;
    pool_sizes.reserve(bindings.size());

    for (const auto& b : bindings)
    {
      VkDescriptorPoolSize ps{};
      ps.type            = b.type;
      ps.descriptorCount = b.count * set_count;
      pool_sizes.push_back(ps);
    }

    VkDescriptorPoolCreateInfo info{};
    info.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    info.poolSizeCount = static_cast<uint32_t>(pool_sizes.size());
    info.pPoolSizes    = pool_sizes.data();
    info.maxSets       = set_count;

    if (vkCreateDescriptorPool(m_device, &info, nullptr, &m_pool) != VK_SUCCESS)
      LOG_ERR("[DescriptorSet] failed to create pool");

    LOG_INFO("[DescriptorSet] pool created");
  }

  void DescriptorSet::allocate_sets(uint32_t set_count)
  {
    std::vector<VkDescriptorSetLayout> layouts(set_count, m_layout);

    VkDescriptorSetAllocateInfo info{};
    info.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    info.descriptorPool     = m_pool;
    info.descriptorSetCount = set_count;
    info.pSetLayouts        = layouts.data();

    m_sets.resize(set_count);
    if (vkAllocateDescriptorSets(m_device, &info, m_sets.data()) != VK_SUCCESS)
      LOG_ERR("[DescriptorSet] failed to allocate sets");

    LOG_INFO("[DescriptorSet] sets allocated");
  }

  void DescriptorSet::update(uint32_t frame, uint32_t binding,
      VkDescriptorType type,
      const VkDescriptorBufferInfo& buffer_info)
  {
    VkWriteDescriptorSet write{};
    write.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.dstSet          = m_sets[frame];
    write.dstBinding      = binding;
    write.dstArrayElement = 0;
    write.descriptorType  = type;
    write.descriptorCount = 1;
    write.pBufferInfo     = &buffer_info;

    vkUpdateDescriptorSets(m_device, 1, &write, 0, nullptr);
  }

  void DescriptorSet::update(uint32_t frame, uint32_t binding,
      VkDescriptorType type,
      const VkDescriptorImageInfo& image_info)
  {
    VkWriteDescriptorSet write{};
    write.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.dstSet          = m_sets[frame];
    write.dstBinding      = binding;
    write.dstArrayElement = 0;
    write.descriptorType  = type;
    write.descriptorCount = 1;
    write.pImageInfo      = &image_info;

    vkUpdateDescriptorSets(m_device, 1, &write, 0, nullptr);
  }
}
