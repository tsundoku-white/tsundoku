#pragma once

// types
#include <cstdint>
#include <print>
#include <set>

// vulkan
#define VULKAN_HPP_NO_EXCEPTIONS
#include <vulkan/vulkan.h>

// math
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/common.hpp>
#include <glm/gtc/quaternion.hpp>

// constants
constexpr uint32_t FRAMES_IN_FLIGHT = 2;
constexpr uint32_t FRAME_CAP        = 200;
constexpr bool     VSYNC            = true;

// logging
#ifdef NDEBUG
  #define LOG_DEBUG(fmt, ...) ((void)0)
#else
  #define LOG_DEBUG(fmt, ...) std::println("\033[35m[DEBUG]\033[0m " fmt, ##__VA_ARGS__) // magenta
#endif
  #define LOG_INFO(fmt, ...)  std::println("\033[34m[INFO ]\033[0m " fmt, ##__VA_ARGS__) // blue
  #define LOG_WARN(fmt, ...)  std::println("\033[33m[WARN ]\033[0m " fmt, ##__VA_ARGS__) // yellow
  #define LOG_PASS(fmt, ...)  std::println("\033[32m[PASS ]\033[0m " fmt, ##__VA_ARGS__) // green
#define LOG_ERR(fmt, ...) \
  do { \
    std::println("\033[31m[ERROR]\033[0m " fmt, ##__VA_ARGS__); \
    throw std::runtime_error("fatal error"); \
  } while(0)

#define LOG_PROGRESS(current, total, label)                                \
  do {                                                                     \
    constexpr int BAR_WIDTH = 20;                                          \
    int pct    = (int)(100.0f * (current) / (total));                      \
    int filled = (int)(BAR_WIDTH * (current) / (total));                   \
    if ((current) == 0) std::print("\033[?25l");  /* hide cursor */        \
    std::print("\r\033[34m[LOAD ]\033[0m {} [", label);                    \
    for (int _i = 0; _i < BAR_WIDTH; _i++)                                 \
      std::print("{}", _i < filled ? "\u25AE" : "\u25AF");                 \
    std::print("] {:3d}%", pct);                                           \
    if ((current) >= (total)) {                                            \
      std::println("");                                                    \
      std::print("\033[?25h");                                             \
    }                                                                      \
  } while(0)
