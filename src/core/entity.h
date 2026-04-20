#pragma once

#include "core/common.h"
#include <algorithm>
#include <cstdint>
#include <glm/detail/type_quat.hpp>
#include <glm/ext/quaternion_trigonometric.hpp>
#include <glm/vec3.hpp>
#include <typeindex>
#include <unordered_map>
#include <memory>

namespace tsundoku
{
  class Entity;

  class Component 
  {
    public:
      Component() = default;
      virtual ~Component() = default;

      Component(const Component&) = delete;
      Component& operator = (const Component&) = delete;

      Entity* owner = nullptr;

      virtual void update(float delta_time) {};
      virtual void start() {};
      virtual void end() {};
  };

  class Transform : public Component
  {
    public:
      glm::vec3 location = {0,0,0};
      glm::vec3 scale    = {1,1,1};
      glm::quat rotation = glm::quat(1.f, 0.f, 0.f, 0.f);
      float pitch = 0, yaw = 0, roll = 0;

      glm::vec3 get_forward() const { return rotation * glm::vec3(0.f, 0.f, -1.f); }
      glm::vec3 get_right()   const { return rotation * glm::vec3(1.f, 0.f, 0.f); }
      glm::vec3 get_up()      const { return rotation * glm::vec3(0.f, 1.f, 0.f); }

      void update(float delta_time) override
      { 
        add_rotation(pitch, yaw, roll);
        pitch = yaw = roll = 0;
      }


    private:
      void add_rotation(float pitch_deg, float yaw_deg, float roll_deg)
      {
        glm::quat world_yaw   = glm::angleAxis(glm::radians(yaw_deg),   glm::vec3(0.f, 1.f, 0.f));
        glm::quat local_pitch = glm::angleAxis(glm::radians(pitch_deg), glm::vec3(1.f, 0.f, 0.f));
        rotation = world_yaw * rotation * local_pitch;
      }
  };

  class Entity 
  {
    public:
      Entity();
      ~Entity();

      Entity(const Entity&) = delete;
      Entity& operator = (const Entity&) = delete;

      uint32_t id = 0;
      Transform transform;

      template<typename T, typename... Args>
        void add_component(Args&&... args)
        {
          auto component = std::make_unique<T>(std::forward<Args>(args)...);
          component->owner = this;
          m_components[std::type_index(typeid(T))] = std::move(component);
          LOG_INFO("[Entity] added: {}", typeid(T).name());
        }

      template<typename T>
        bool has_component() const
        {
          return m_components.count(std::type_index(typeid(T))) > 0;
        }

      template<typename T>
        T* get_component()
        {
          auto item = m_components.find(std::type_index(typeid(T)));
          if (item != m_components.end())
            return static_cast<T*>(item->second.get());
        LOG_WARN("[Entity] does not exist: {}", typeid(T).name());
        return nullptr;
      }

      const std::unordered_map<std::type_index, std::unique_ptr<Component>>& get_components() const 
      { 
        return m_components; 
      }

      void set_parent(Entity* parent)
      {
        if (m_parent)
        {
          auto& siblings = m_parent->m_children;
          siblings.erase(std::remove(siblings.begin(), siblings.end(), this), siblings.end());
        }
        m_parent = parent;
        if (parent)
          parent->m_children.push_back(this);
      }

      Entity* get_parent() { return m_parent; }
      const std::vector<Entity*>& get_children() { return m_children; }

      glm::vec3 get_world_position()
      { 
        glm::vec3 world_pos = transform.location;
        Entity* current = m_parent;
        while (current)
        {
          world_pos += current->transform.location;
          current = current->get_parent();
        }
        return world_pos;
      }

    private:
      std::unordered_map<std::type_index, std::unique_ptr<Component>> m_components;
      Entity* m_parent = nullptr;
      std::vector<Entity*> m_children;
  };
}
