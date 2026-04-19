#include "entity.h"

namespace tsundoku
{
  Entity::Entity()
  {
    static uint32_t next_id = 1;
    id = next_id++;
    LOG_INFO("[Entity] Created with ID: {}", id);
  }

  Entity::~Entity() { 
    LOG_INFO("[Entity] Destroyed ID's");
    m_components.clear(); 
  }
}
