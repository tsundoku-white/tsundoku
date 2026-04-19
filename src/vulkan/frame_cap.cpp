#include "frame_cap.h"
#include <thread>

namespace tsundoku
{
  using namespace std::chrono;

  void FrameCap::start()
  {
    m_start_time = clock::now();
  }

  void FrameCap::end()
  {
    constexpr auto k_target = std::chrono::duration<double>(1.0 / FRAME_CAP);
    auto elapsed = clock::now() - m_start_time;

    if (elapsed < k_target) {
      volatile int dummy = 0; 
      while (clock::now() - m_start_time < k_target) {
        (void)dummy;
      }
    }

    delta_time = std::chrono::duration<float>(clock::now() - m_start_time).count();
  }
}
