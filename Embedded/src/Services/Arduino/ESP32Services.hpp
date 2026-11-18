#pragma once
#include "iServices.hpp"
#include "ESP32ResetReason.hpp"
#include "ESP32WiFi.hpp"
#include "ESP32WebServer.hpp"

class ESP32Services : public iServices
{
public:
  void Init(void) override
  {
    ESP32ResetReason().print();
    if (Configuration.isWiFiEnabled)
    {
      esp32WiFi.setup();
    }
    if (Configuration.isWebServerEnabled)
    {
      esp32WebServer.Init();
    }
    isInitialized = true;
  }

  void Poll(void) override
  {
    if (Configuration.isWiFiEnabled)
    {
      esp32WiFi.run();
    }
    if (Configuration.isWebServerEnabled)
    {
      esp32WebServer.Poll();
    }
  }

private:
  ESP32WiFi esp32WiFi           = ESP32WiFi();
  ESP32WebServer esp32WebServer = ESP32WebServer();
};
