#include "core/common.h"
#include <glm/fwd.hpp>
#include "core/platform.h"

#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace tsundoku
{
  Platform::Platform()
  {
    if (!glfwInit())
    {
      LOG_ERR("[Platform] failed to load glfw");
      return;
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    m_handle = glfwCreateWindow(m_width, m_height, m_title.c_str(), nullptr, nullptr);
    if (!m_handle)
    {
      LOG_ERR("[Platform] failed to create window");
      glfwTerminate();
      return;
    }
    glfwSetWindowUserPointer(m_handle, this);
    glfwSetFramebufferSizeCallback(m_handle, framebufferResizeCallback);
    set_resized(false);
    glfwSetCursorPosCallback(m_handle, mouseMoveCallback);
    glfwSetInputMode(m_handle, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    double initX, initY;
    glfwGetCursorPos(m_handle, &initX, &initY);
    mouse_last_x = initX;
    mouse_last_y = initY;
    mouse_x = 0;
    mouse_y = 0;
  }

  void Platform::framebufferResizeCallback(GLFWwindow* window, int width, int height)
  {
    auto* platform = reinterpret_cast<Platform*>(glfwGetWindowUserPointer(window));
    platform->set_resized(true);
  }

  void Platform::mouseMoveCallback(GLFWwindow* window, double xpos, double ypos)
  {
    auto* platform = reinterpret_cast<Platform*>(glfwGetWindowUserPointer(window));
    platform->mouse_x = xpos - platform->mouse_last_x;
    platform->mouse_y = ypos - platform->mouse_last_y;
    platform->mouse_last_x = xpos;
    platform->mouse_last_y = ypos;
  }

  Platform::~Platform()
  {
    glfwTerminate();
  }

  void Platform::poll_events()
  {

    glfwPollEvents();
  }

  void Platform::set_should_close(bool value)
  {
    glfwSetWindowShouldClose(m_handle, value);
  }

  bool Platform::should_close()
  {
    return glfwWindowShouldClose(m_handle);
  }

  bool Platform::key_pressed(KeyMap key)
  {
    return glfwGetKey(m_handle, key);
  }

  glm::vec2 Platform::get_mouse_delta()
  {
    glm::vec2 delta(mouse_x, mouse_y);
    mouse_x = 0;
    mouse_y = 0;
    return delta;
  }

  GLFWwindow *Platform::get_handle()
  {
    return m_handle;
  }
}
