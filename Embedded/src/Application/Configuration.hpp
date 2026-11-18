#pragma once
// Build-time configuration. Runtime-tunable state (theme, brightness, power)
// lives in the theme engine and is persisted via the theme store.
#include "Units.hpp"

#ifndef LED_DATA_PIN
  #define LED_DATA_PIN 5 // GPIO driving the WS2811 data line
#endif

#ifndef LED_COUNT
  #define LED_COUNT 50 // pixels on the strip
#endif

class ConfigurationSingleton
{
public:
  static ConfigurationSingleton& GetConfigurationSingleton(void)
  {
    static ConfigurationSingleton singleton;
    return singleton;
  }

private:
  ConfigurationSingleton(void)                                = default;
  ~ConfigurationSingleton(void)                               = default;
  ConfigurationSingleton(const ConfigurationSingleton&)        = delete;
  ConfigurationSingleton& operator=(const ConfigurationSingleton&) = delete;

public:
#ifdef PLATFORM_ARDUINO
  static constexpr uint16_t HTTPPort = 80;
#else
  static constexpr uint16_t HTTPPort = 8080;
#endif
  static constexpr Frequency ServicesPollingRate = 100_hertz;
  static constexpr Time ServicesPollingInterval  = 1 / ServicesPollingRate;
  static constexpr Frequency LightsPollingRate   = 30_hertz;
  static constexpr Time LightsPollingInterval    = 1 / LightsPollingRate;
  static constexpr uint8_t LEDDataPin            = LED_DATA_PIN;
  static constexpr uint16_t LEDPixelCount        = LED_COUNT;

  // Service Enablement
  static constexpr bool isWebServerEnabled = true;
#ifdef PLATFORM_ARDUINO
  static constexpr bool isWiFiEnabled = true;
#endif
};

static ConfigurationSingleton& Configuration = ConfigurationSingleton::GetConfigurationSingleton();
