#pragma once

#include "core/entity.h"
#include <sys/types.h>
namespace tsundoku
{
  class Camera : public Component
  {
    public:
      Camera() = default;
      ~Camera() = default;

      Transform transform = {};
      bool is_primary = false;

      float field_of_view = 65.f;
      float near_clipping_plane = 0.05f;
      float far_clipping_plane = 4'000.f;

      glm::mat4 projection_matrix;
      glm::mat4 view_matrix = glm::lookAt(
          glm::vec3(0.f, 0.f, 2.f),
          glm::vec3(0.f, 0.f, 0.f),
          glm::vec3(0.f, 1.f, 0.f)
          );

    private:
      void update(float delta_time) override;
      void start() override;
      void end() override;
  };
}
