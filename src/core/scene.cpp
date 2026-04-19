#include "scene.h"

namespace tsundoku
{
  Scene::Scene()
  {
    LOG_INFO("[Scene] created");
  }

  Scene::~Scene()
  {
    m_entities.clear();
    LOG_INFO("[Scene] destroyed");
  }

  Entity& Scene::create_entity()
  {
    auto entity        = std::make_unique<Entity>();
    uint32_t id        = entity->id;
    auto& ref          = *entity;
    m_entities[id]     = std::move(entity);
    return ref;
  }

  Entity* Scene::get_entity(uint32_t id)
  {
    auto item = m_entities.find(id);
    if (item != m_entities.end())
      return item->second.get();
    LOG_WARN("[Scene] entity {} does not exist", id);
    return nullptr;
  }

  void Scene::destroy_entity(uint32_t id)
  {
    auto item = m_entities.find(id);
    if (item != m_entities.end())
    {
      m_entities.erase(item);
      return;
    }
    LOG_WARN("[Scene] tried to destroy entity {} that does not exist", id);
  }

  const std::unordered_map<uint32_t, std::unique_ptr<Entity>>& Scene::get_entities() const
  {
    return m_entities;
  }

  void Scene::load(const std::string& path)
  {
    // TODO: open binary file at path
    // read header (magic + version)
    // for each entity block: create_entity(), then deserialize components
    LOG_WARN("[Scene] load not implemented");
  }

  void Scene::save(const std::string& path) const
  {
    // TODO: open binary file at path for writing
    // write header (magic + version)
    // for each entity in m_entities: serialize id + components
    LOG_WARN("[Scene] save not implemented");
  }
}
