#pragma once
#include "core/common.h"

namespace tsundoku
{
  struct DescriptorBinding
  {
    uint32_t             binding;
    VkDescriptorType     type;
    VkShaderStageFlags   stage;
    uint32_t             count = 1;
  };

  class DescriptorSet
  {
    public:
      DescriptorSet() = default;
      DescriptorSet(VkDevice device, const std::vector<DescriptorBinding>& bindings, uint32_t set_count);
      ~DescriptorSet();

      DescriptorSet(const DescriptorSet&)            = delete;
      DescriptorSet& operator=(const DescriptorSet&) = delete;

      DescriptorSet(DescriptorSet&& other) noexcept;
      DescriptorSet& operator=(DescriptorSet&& other) noexcept;

      VkDescriptorSetLayout get_layout()             const { return m_layout; }
      VkDescriptorSet       get_set(uint32_t frame)  const { return m_sets[frame]; }

      void update(uint32_t frame, uint32_t binding, VkDescriptorType type, const VkDescriptorBufferInfo& buffer_info);
      void update(uint32_t frame, uint32_t binding, VkDescriptorType type, const VkDescriptorImageInfo&  image_info);

    private:
      VkDevice              m_device = VK_NULL_HANDLE;
      VkDescriptorSetLayout m_layout = VK_NULL_HANDLE;
      VkDescriptorPool      m_pool   = VK_NULL_HANDLE;
      std::vector<VkDescriptorSet> m_sets;

      void create_layout(const std::vector<DescriptorBinding>& bindings);
      void create_pool  (const std::vector<DescriptorBinding>& bindings, uint32_t set_count);
      void allocate_sets(uint32_t set_count);
  };
}
