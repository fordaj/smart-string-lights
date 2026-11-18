#pragma once
#include "ThemeEngine.hpp"
#include <cstddef>

class iLEDStrip
{
public:
  virtual ~iLEDStrip()                                     = default;
  virtual void Init(void)                                  = 0;
  virtual size_t PixelCount(void) const                    = 0;
  virtual void Render(const Color* pixels, size_t count)     = 0; // writes every pixel, then latches
};
