#pragma once

#include "iLEDStrip.hpp"
#ifdef PLATFORM_ARDUINO
  #ifdef EMULATED
    #include "EmulatedStrip.hpp"
using StripImpl = EmulatedStrip;
  #else
    #include "WS2811Strip.hpp"
using StripImpl = WS2811Strip;
  #endif
#elif defined(PLATFORM_PC)
  #include "AnsiStrip.hpp"
using StripImpl = AnsiStrip;
#endif

class PeripheralsSingleton
{
public:
  static PeripheralsSingleton& GetPeripheralsSingleton(void)
  {
    static PeripheralsSingleton singleton;
    return singleton;
  }

private:
  PeripheralsSingleton(void)                              = default;
  ~PeripheralsSingleton(void)                             = default;
  PeripheralsSingleton(const PeripheralsSingleton&)       = delete;
  PeripheralsSingleton& operator=(const PeripheralsSingleton&) = delete;

public:
  void Init(void)
  {
    strip.Init();
  }

  iLEDStrip& GetStrip(void)
  {
    return strip;
  }

private:
  StripImpl strip = StripImpl();
};

static PeripheralsSingleton& Peripherals = PeripheralsSingleton::GetPeripheralsSingleton();
