
#pragma once

#include "iResetReason.hpp"
#include <WiFi.h>
#include <esp_system.h>

class ESP32ResetReason : public IResetReason
{
public:
  void print() const override
  {
    esp_reset_reason_t reason = esp_reset_reason();
    Serial.print("Reset reason: ");
    Serial.println(toString(reason));
  }

private:
  const char* toString(esp_reset_reason_t reason) const
  {
    switch (reason)
    {
      case ESP_RST_UNKNOWN:
        return "Unknown";
      case ESP_RST_POWERON:
        return "Power-on";
      case ESP_RST_EXT:
        return "External pin";
      case ESP_RST_SW:
        return "Software reset";
      case ESP_RST_PANIC:
        return "Exception/Panic";
      case ESP_RST_INT_WDT:
        return "Interrupt watchdog";
      case ESP_RST_TASK_WDT:
        return "Task watchdog";
      case ESP_RST_WDT:
        return "Other watchdog";
      case ESP_RST_DEEPSLEEP:
        return "Wake from deep sleep";
      case ESP_RST_BROWNOUT:
        return "Brownout";
      case ESP_RST_SDIO:
        return "SDIO";
      default:
        return "Unhandled";
    }
  }
};
