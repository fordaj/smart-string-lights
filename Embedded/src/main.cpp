#include "Lights.hpp"
#include "Platform.hpp"
#include "Scheduler.hpp"
#include "Services.hpp"
#include "Configuration.hpp"
#include "Peripherals.hpp"
#include "Version.hpp"
#ifdef PLATFORM_ARDUINO
#include <Arduino.h>
#else
#include <unistd.h>
#endif // PLATFORM

void PollServices()
{
  Services.Poll();
}

void PollLights()
{
  Lights.Poll();
}

Task tasks[] = {
  // clang-format off
  { PollServices, uint32_t(Configuration.ServicesPollingInterval.Milliseconds()), 0 },
  { PollLights, uint32_t(Configuration.LightsPollingInterval.Milliseconds()), 0 },
  // clang-format on
};

Scheduler scheduler(tasks, sizeof(tasks) / sizeof(Task));

void setup()
{
#ifdef PLATFORM_ARDUINO
  Serial.begin(115200);
  delay(1000); // give the USB CDC serial a moment to enumerate
#endif
  logger.logf("smart-string-lights\n");
  logger.logf("Version: %u.%u.%u.%u.%u\n", Version::branch, Version::major, Version::minor, Version::revision, Version::patch);

  Peripherals.Init();
  Lights.Init();
  Services.Init();
  scheduler.Init();
}

void loop()
{
  scheduler.RunOnce();
}

#if defined(PLATFORM_PC)
int main()
{
  setup();
  while (1)
  {
    loop();
  }
  return 0;
}
#endif // PLATFORM
