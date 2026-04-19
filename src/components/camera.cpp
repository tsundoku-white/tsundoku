#include "camera.h"
#include <glm/gtc/matrix_transform.hpp>

namespace tsundoku
{
  void Camera::start() { return; }
  void Camera::update(float delta_time)
  { 
    view_matrix = glm::lookAt(transform.location, 
        transform.location + transform.get_forward(), transform.get_up());
  }
  void Camera::end() { return; }
}
