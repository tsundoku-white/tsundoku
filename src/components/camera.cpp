#include "camera.h"
#include <glm/gtc/matrix_transform.hpp>

namespace tsundoku
{
  void Camera::start() { return; }
  void Camera::update(float delta_time)
  {
    glm::vec3 world_pos = owner->transform.location + transform.location;
    glm::quat world_rot = owner->transform.rotation * transform.rotation;

    glm::vec3 forward = world_rot * glm::vec3(0.f, 0.f, -1.f);
    glm::vec3 up      = world_rot * glm::vec3(0.f, 1.f, 0.f);

    view_matrix = glm::lookAt(world_pos, world_pos + forward, up);
  }
  void Camera::end() { return; }
}
