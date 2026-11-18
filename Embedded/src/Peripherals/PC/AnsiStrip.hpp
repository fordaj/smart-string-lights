#pragma once
#include <cstdio>

#include "iLEDStrip.hpp"
#include "Configuration.hpp"

// Host-side stand-in for the WS2811 string: paints the strip as a line of
// truecolor ANSI blocks in the terminal so theme changes are visible while
// developing without hardware.
class AnsiStrip : public iLEDStrip
{
public:
  void Init(void) override
  {
    std::printf("smart-string-lights host preview — %u pixels, GPIO%u (virtual)\n", Configuration.LEDPixelCount, Configuration.LEDDataPin);
  }

  size_t PixelCount(void) const override
  {
    return Configuration.LEDPixelCount;
  }

  void Render(const Color* pixels, size_t count) override
  {
    std::fputs("\r", stdout);
    for (size_t i = 0; i < count; ++i)
    {
      std::printf("\x1b[48;2;%u;%u;%um  ", pixels[i].r, pixels[i].g, pixels[i].b);
    }
    std::fputs("\x1b[0m ", stdout);
    std::fflush(stdout);
  }
};
