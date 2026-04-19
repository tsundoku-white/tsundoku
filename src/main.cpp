#include "core/platform.h"
#include "core/scene.h"
#include "components/camera.h"
#include "vulkan/render_system.h"

using namespace tsundoku;

int main()
{
  Scene scene;

  Entity& viewport = scene.create_entity();
  viewport.add_component<Camera>();
  Camera* camera = viewport.get_component<Camera>();
  camera->is_primary = true;
  camera->transform.location = {0, 0, 2};

  Platform platform;
  RenderSystem render_system(platform, *camera);

  float speed = 15.f;

  auto viewport_move = [&]()
  {
    float delta = render_system.delta_time;

    if (platform.key_pressed(KEY_A))
      camera->transform.location -= camera->transform.get_right() * speed * delta;
    if (platform.key_pressed(KEY_D))
      camera->transform.location += camera->transform.get_right() * speed * delta;
    if (platform.key_pressed(KEY_W))
      camera->transform.location += camera->transform.get_forward() * speed * delta;
    if (platform.key_pressed(KEY_S))
      camera->transform.location -= camera->transform.get_forward() * speed * delta;
  };

  while (!platform.should_close())
  {
    platform.poll_events();

    if (platform.key_pressed(KEY_ESCAPE))
      platform.set_should_close(true);

    viewport_move();
    render_system.frame_pass(scene);
  }
}
