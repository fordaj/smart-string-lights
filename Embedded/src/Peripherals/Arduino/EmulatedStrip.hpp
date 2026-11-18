#pragma once
#include <Arduino.h>

#include "iLEDStrip.hpp"
#include "Configuration.hpp"

// Strip stand-in for ESP32_S3_EMULATED builds: no FastLED, no GPIO timing —
// each rendered frame is dumped to Serial as run-length-encoded pixel colors,
// so themes can be verified on real hardware without LEDs attached.
class EmulatedStrip : public iLEDStrip
{
public:
  void Init(void) override
  {
    Serial.printf("EmulatedStrip: %u pixels on virtual GPIO%u (EMULATED build; no output pin driven)\n", Configuration.LEDPixelCount, Configuration.LEDDataPin);
  }

  size_t PixelCount(void) const override
  {
    return Configuration.LEDPixelCount;
  }

  void Render(const Color* pixels, size_t count) override
  {
    Serial.print("frame:");
    size_t i = 0;
    while (i < count)
    {
      size_t run = 1;
      while (i + run < count && pixels[i + run] == pixels[i])
      {
        ++run;
      }
      Serial.printf(" [%zu-%zu %s]", i, i + run - 1, rgbToHex(pixels[i]).c_str());
      i += run;
    }
    Serial.println();
  }
};
