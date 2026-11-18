#pragma once
#include <FastLED.h>

#include "iLEDStrip.hpp"
#include "Configuration.hpp"

// WS2811 string (5 V or 12 V; 800 kHz protocol, GRB order).
// GRB is the vast majority of WS2811 strips; change here if yours is different.
class WS2811Strip : public iLEDStrip
{
public:
  void Init(void) override
  {
    FastLED.addLeds<WS2811, Configuration.LEDDataPin, GRB>(leds, Configuration.LEDPixelCount);
    FastLED.clear(true);
    FastLED.show();
  }

  size_t PixelCount(void) const override
  {
    return Configuration.LEDPixelCount;
  }

  void Render(const Color* pixels, size_t count) override
  {
    for (size_t i = 0; i < count && i < Configuration.LEDPixelCount; ++i)
    {
      leds[i] = CRGB(pixels[i].r, pixels[i].g, pixels[i].b);
    }
    FastLED.show();
  }

private:
  CRGB leds[Configuration.LEDPixelCount];
};
