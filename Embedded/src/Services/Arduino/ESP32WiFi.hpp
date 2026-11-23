#pragma once

#include <WiFi.h>
#include <WiFiManager.h>
#include <Preferences.h>
#include <ESPmDNS.h>
#include "Configuration.hpp"

class ESP32WiFi
{
public:
  void setup()
  {
    uint64_t chipid = ESP.getEfuseMac();
    snprintf(hostname, sizeof(hostname), "SmartLights-%06llX", chipid & 0xFFFFFF);

    Serial.printf("Hostname: %s\n", hostname);

    WiFi.setHostname(hostname);
    wm.setConfigPortalBlocking(false);
    wm.setConnectTimeout(WIFI_CONNECT_GRACE_SECONDS);

    // Load our own persistent credentials from NVS.
    preferences.begin("wifi", true);

    String ssid     = preferences.getString("ssid", "");
    String password = preferences.getString("password", "");

    preferences.end();

    bootMillis_ = millis();

    // Serving HTTP from a power-save WiFi is a terrible experience (huge
    // latency, dropped packets); keep the radio on.
    WiFi.setSleep(false);

    if (ssid.length() > 0)
    {
      Serial.printf("Found saved WiFi credentials for SSID: %s\n", ssid.c_str());

      WiFi.mode(WIFI_STA);
      WiFi.begin(ssid.c_str(), password.c_str());

      // We have credentials, so don't immediately start the portal.
      // run() will monitor the connection and start WiFiManager if
      // the connection fails.
      return;
    }

    Serial.println("No saved WiFi credentials");

    // No credentials yet, so let WiFiManager start the portal.
  }

  void run()
  {
    // Service the WiFiManager portal if it is running.
    wm.process();

    // Already connected.
    if (WiFi.status() == WL_CONNECTED)
    {
      if (!wasConnected)
      {
        wasConnected = true;
        hadConnection = true;

        setupMDNS();

        Serial.println("Connected!");
        Serial.print("IP: ");
        Serial.println(WiFi.localIP());
      }

      return;
    }

    // We lost the connection.
    if (wasConnected)
    {
      wasConnected = false;
      Serial.println("WiFi disconnected");
    }

    // Don't repeatedly start the portal.
    if (portalStarted)
    {
      return;
    }

    // Boot grace period: a fresh WiFi.begin() usually needs 2-10 s to
    // associate. Without this, the first poll (immediately after setup())
    // declares failure and starts the portal, which only the portal's
    // autoConnect then has to rescue — leaving the device off the LAN
    // whenever that fails. Wait before falling back to the portal.
    if (!hadConnection && millis() - bootMillis_ < Configuration.WiFiConnectGraceMs)
    {
      if (millis() - lastWaitLog_ > 3000)
      {
        Serial.println("Waiting for WiFi to connect...");
        lastWaitLog_ = millis();
      }
      return;
    }

    Serial.println("WiFi connection failed; starting configuration portal...");

    portalStarted = true;

    wm.setSaveConfigCallback(saveCredentials);

    if (wm.autoConnect(hostname))
    {
      // Connection succeeded immediately.
      portalStarted = false;
      wasConnected  = true;

      Serial.println("Connected!");
      Serial.print("IP: ");
      Serial.println(WiFi.localIP());

      setupMDNS();
    }
    else
    {
      // Non-blocking mode means the portal is now being serviced
      // by wm.process().
      Serial.println("WiFi configuration portal started");
    }
  }

  bool connected() const
  {
    return WiFi.status() == WL_CONNECTED;
  }

private:
  static void saveCredentials()
  {
    String ssid     = WiFi.SSID();
    String password = WiFi.psk();

    Serial.printf("Saving WiFi credentials for SSID: %s\n", ssid.c_str());

    preferences.begin("wifi", false);

    preferences.putString("ssid", ssid);
    preferences.putString("password", password);

    preferences.end();

    Serial.println("WiFi credentials saved to NVS");
  }

  void setupMDNS()
  {
    if (mdnsStarted)
    {
      return;
    }

    if (!MDNS.begin(hostname))
    {
      Serial.println("Failed to start mDNS");
      return;
    }

    MDNS.addService("http", "tcp", 80);

    mdnsStarted = true;

    Serial.println("mDNS responder started");
    Serial.printf("http://%s.local:%hu/\n", hostname, Configuration.HTTPPort);
  }

  char hostname[32] = {};

  bool portalStarted = false;
  bool wasConnected  = false;
  bool hadConnection = false;
  bool mdnsStarted   = false;
  uint32_t bootMillis_ = 0;
  uint32_t lastWaitLog_ = 0;

  inline static WiFiManager wm;
  inline static Preferences preferences;
};