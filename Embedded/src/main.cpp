#include <Arduino.h>

#include "Version.hpp"

void setup()
{
  Serial.begin(115200);
  delay(1000); // give the USB CDC serial a moment to enumerate

  Serial.println("Hello, smart-string-lights!");
  Serial.printf("Version: %u.%u.%u.%u.%u\n",
                Version::branch,
                Version::major,
                Version::minor,
                Version::revision,
                Version::patch);
}

void loop()
{
  delay(1000);
}
