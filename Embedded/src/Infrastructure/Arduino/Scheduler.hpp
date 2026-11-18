#include <Arduino.h>
#include "iScheduler.hpp"

namespace Platform
{

  Tick Now()
  {
    return millis();
  }

  void Idle()
  {
    delay(1); // yields to ESP32 background tasks
  }

} // namespace Platform