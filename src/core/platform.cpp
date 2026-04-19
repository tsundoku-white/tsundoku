#include "core/common.h"
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
  }

  void Platform::framebufferResizeCallback(GLFWwindow* window, int width, int height)
  {
    auto* platform = reinterpret_cast<Platform*>(glfwGetWindowUserPointer(window));
    platform->set_resized(true);
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

  GLFWwindow *Platform::get_handle()
  {
    return m_handle;
  }
}
