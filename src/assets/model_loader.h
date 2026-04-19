#pragma once
#include "core/common.h"

struct cgltf_data;

namespace tsundoku
{
  class VkContext;
  class Model_Loader 
  {
    public:
      Model_Loader(VkContext &context, std::string path);
      ~Model_Loader();

      glm::vec3 pos;
      glm::vec3 normal;
      glm::vec2 uv;

      uint32_t vertex_count  = 0;
      uint32_t indices_count = 0;
    private:
      cgltf_data* m_data        = nullptr;
  };
}
