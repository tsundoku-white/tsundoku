#pragma once

#include "core/common.h"
#include <cstdint>
#include <memory>
#include <unordered_map>
#include "entity.h"

namespace tsundoku
{
  class Scene
  {
    public:
      Scene();
      ~Scene();

      Scene(const Scene&) = delete;
      Scene& operator=(const Scene&) = delete;

      Entity&  create_entity();
      Entity*  get_entity(uint32_t id);
      void     destroy_entity(uint32_t id);

      const std::unordered_map<uint32_t, std::unique_ptr<Entity>>& get_entities() const;

      void load(const std::string& path);
      void save(const std::string& path) const;

    private:
      std::unordered_map<uint32_t, std::unique_ptr<Entity>> m_entities;
  };
}
