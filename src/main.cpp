#include "core/platform.h"
#include "core/scene.h"
#include "components/camera.h"
#include "components/model.h"
#include "vulkan/render_system.h"
#include <print>

using namespace tsundoku;

int main()
{
  Scene scene;

  Entity& viewport = scene.create_entity();
  viewport.add_component<Camera>();
  Camera* camera = viewport.get_component<Camera>();
  camera->is_primary = true;
  camera->transform.location = {0, 0, 2};

  for (int x = 0; x < 10; x++)
    for (int z = 0; z < 10; z++)
    {
      Entity& obj = scene.create_entity();
      obj.add_component<Model>(ASSET_PATH "models/cube.glb");
      obj.transform.location = {x * 2.f, 0.f, z * 2.f};
    }

  Platform platform;
  RenderSystem render_system(platform, *camera);

  float speed = 15.f;
  float sence = 0.1f; 

  auto viewport_move = [&]()
  {
    float delta = render_system.delta_time;

    glm::vec2 mouse = platform.get_mouse_delta();
    viewport.transform.yaw   = -mouse.x * sence;
    viewport.transform.pitch = -mouse.y * sence;

    if (platform.key_pressed(KEY_A))
      viewport.transform.location -= viewport.transform.get_right() * speed * delta;
    if (platform.key_pressed(KEY_D))
      viewport.transform.location += viewport.transform.get_right() * speed * delta;
    if (platform.key_pressed(KEY_W))
      viewport.transform.location += viewport.transform.get_forward() * speed * delta;
    if (platform.key_pressed(KEY_S))
      viewport.transform.location -= viewport.transform.get_forward() * speed * delta;
    if (platform.key_pressed(KEY_LEFT_SHIFT))
      viewport.transform.location -= viewport.transform.get_up() * speed * delta;
    if (platform.key_pressed(KEY_SPACE))
      viewport.transform.location += viewport.transform.get_up() * speed * delta;
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
