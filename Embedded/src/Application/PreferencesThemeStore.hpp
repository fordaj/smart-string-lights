#pragma once
// NVS-backed theme store (ESP32). Persists LightsState as a JSON blob in the
// "lights" namespace, key "state".
#include <Preferences.h>
#include <ArduinoJson.h>
#include <string>
#include <vector>

#include "ThemeEngine.hpp"

class PreferencesThemeStore : public iThemeStore
{
public:
  bool Load(LightsState& out) override
  {
    // Read-write open so a missing namespace doesn't make the core log a
    // spurious "nvs_open failed: NOT_FOUND" before the first save.
    Preferences prefs;
    prefs.begin("lights", false);
    String blob = prefs.getString("state", "");
    prefs.end();
    if (blob.length() == 0)
    {
      return false;
    }

    JsonDocument doc;
    if (deserializeJson(doc, blob) != DeserializationError::Ok)
    {
      return false;
    }

    out.power                 = doc["power"] | true;
    out.brightness            = doc["brightness"] | uint8_t(128);
    out.activeTheme           = doc["activeTheme"] | std::string("Candy Cane");
    out.customThemes.clear();
    for (JsonObject t : doc["customThemes"].as<JsonArray>())
    {
      Theme theme;
      theme.name    = std::string(t["name"] | "");
      std::string modeName = t["mode"] | "solid";
      ThemeMode mode;
      theme.mode    = ThemeModeFromName(modeName, mode) ? mode : ThemeMode::Solid;
      theme.builtin = false;
      for (const char* hex : t["colors"].as<JsonArray>())
      {
        Color c{};
        if (hexColorToRGB(std::string(hex ? hex : ""), c))
        {
          theme.colors.push_back(c);
        }
      }
      if (!theme.name.empty() && !theme.colors.empty())
      {
        out.customThemes.push_back(theme);
      }
    }
    return true;
  }

  void Save(const LightsState& in) override
  {
    JsonDocument doc;
    doc["power"]       = in.power;
    doc["brightness"]  = in.brightness;
    doc["activeTheme"] = in.activeTheme;
    JsonArray custom   = doc["customThemes"].to<JsonArray>();
    for (const auto& theme : in.customThemes)
    {
      JsonObject t   = custom.add<JsonObject>();
      t["name"]      = theme.name;
      t["mode"]      = ThemeModeName(theme.mode);
      JsonArray cols = t["colors"].to<JsonArray>();
      for (const auto& c : theme.colors)
      {
        cols.add(rgbToHex(c));
      }
    }

    String blob;
    serializeJson(doc, blob);

    Preferences prefs;
    prefs.begin("lights", false);
    prefs.putString("state", blob);
    prefs.end();
  }
};
