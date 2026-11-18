#pragma once

#ifdef PLATFORM_ARDUINO
  #include "Arduino.hpp"
#elif defined(PLATFORM_PC)
  #include "PC.hpp"
#endif // PLATFORM