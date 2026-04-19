#pragma once

#include "core/common.h"
#include <chrono>

namespace tsundoku
{

  class FrameCap
  {
    using clock = std::chrono::steady_clock;
    public:
      FrameCap() = default;
      ~FrameCap() = default;

      FrameCap(const FrameCap&) = delete;
      FrameCap& operator=(const FrameCap&) = delete;

      float delta_time = 0.f;

      void start();
      void end();
    private:
      clock::time_point m_start_time;
  };
}

