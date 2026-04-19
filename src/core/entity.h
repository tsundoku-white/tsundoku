#pragma once

#include "core/common.h"
#include <algorithm>
#include <cstdint>
#include <glm/detail/type_quat.hpp>
#include <glm/ext/quaternion_trigonometric.hpp>
#include <glm/vec3.hpp>
#include <pstl/glue_algorithm_defs.h>
#include <typeindex>
#include <unordered_map>
#include <memory>

// BTW idk what is happening so there is alot of comments 
// so future me can understand it.

namespace tsundoku
{
  class Component 
  {
    public:
      Component() = default;
      virtual ~Component() = default;

      Component(const Component&) = delete;
      Component& operator = (const Component&) = delete;

      virtual void update(float delta_time) {};
      virtual void start() {};
      virtual void end() {};
    private:
  };

  class Transform : public Component
  {
    public:
      glm::vec3 location     = {0,0,0};
      glm::vec3 scale        = {1,1,1};
      glm::quat rotation = glm::quat(1.f, 0.f, 0.f, 0.f);
      float pitch = 0, yaw = 0, roll = 0;

      glm::vec3 get_forward() const
      {
        return rotation * glm::vec3(0.f, 0.f, -1.f);
      }

      glm::vec3 get_right() const
      {
        return rotation * glm::vec3(1.f, 0.f, 0.f);
      }

      glm::vec3 get_up() const
      {
        return rotation * glm::vec3(0.f, 1.f, 0.f);
      }

    private:
      void update(float delta_time) override
      { 
        add_rotation(pitch, yaw, roll);
        // Reset angles after applying
        pitch = yaw = roll = 0;
      }
      void add_rotation(float pitch_deg, float yaw_deg, float roll_deg)
      {
        glm::quat rot = glm::quat(glm::vec3(
              glm::radians(pitch_deg),
              glm::radians(yaw_deg),
              glm::radians(roll_deg)
              ));
        rotation = rot * rotation;
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

      // remove later
      const std::unordered_map<std::type_index, std::unique_ptr<Component>>& get_components() const 
      { 
        return m_components; 
      }


      // template is a generic type
      template<typename T>
        void add_component()
        {
          // create an unique_ptr to avoid copys
          m_components[std::type_index(typeid(T))] = std::make_unique<T>();
          LOG_INFO("[Entity] added: {}", typeid(T).name());
        }

      template<typename T>
        T* get_component()
        {
          // find the template T then if it is at the end loop once again.
          auto item = m_components.find(std::type_index(typeid(T)));
          if (item != m_components.end())
          {
            return static_cast<T*>(item->second.get());
          }
          LOG_WARN("[Entity] does not exist: {}", typeid(T).name());
          return nullptr;
        }

      void set_parent(Entity *parent)
      {
        if (m_parent)
        {
          auto &siblings = m_parent->m_children;
          siblings.erase(std::remove(siblings.begin(), siblings.end(), this), siblings.end());
        }
        m_parent = parent;

        if (parent)
        {
          parent->m_children.push_back(this);
        }
      }

      Entity* get_parent() { return m_parent; }
      const std::vector<Entity*>& get_children() { return m_children; }

      glm::vec3 get_world_position()
      { 
        auto* transform = get_component<Transform>();
        if (!transform) return glm::vec3(0);

        glm::vec3 world_pos = transform->location;
        Entity* current = m_parent;

        while (current)
        {
          auto* parent_transform = current->get_component<Transform>();
          if (parent_transform)
          {
            world_pos += parent_transform->location;
          }
          current = current->get_parent();
        }
        return world_pos;
      }

    private:
      // full map
      std::unordered_map<std::type_index, std::unique_ptr<Component>> m_components;
      Entity *m_parent = nullptr;
      std::vector<Entity*> m_children;
  };
}
