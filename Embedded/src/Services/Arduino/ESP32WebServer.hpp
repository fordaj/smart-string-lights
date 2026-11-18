#pragma once
#include "Units.hpp"
#include "Configuration.hpp"
#include "Lights.hpp"
#include "iWebServer.hpp"
#include "web.hpp"
#include <ArduinoJson.h>
#include <WebServer.h>

inline WebServer server(Configuration.HTTPPort);

class ESP32WebServer : public iWebServer
{
public:
  void Init(void) override
  {
    server.on("/", HTTP_GET,
              []()
              {
                server.send(200, "text/html", String(INDEX_HTML.data(), INDEX_HTML.size()));
              });

    server.on("/style.css", HTTP_GET,
              []()
              {
                server.send(200, "text/css", String(STYLE_CSS.data(), STYLE_CSS.size()));
              });

    server.on("/script.js", HTTP_GET,
              []()
              {
                server.send(200, "application/javascript", String(SCRIPT_JS.data(), SCRIPT_JS.size()));
              });

    server.on("/api/state", HTTP_GET,
              []()
              {
                server.send(200, "application/json", stateJson());
              });

    server.on("/api/state", HTTP_POST,
              []()
              {
                JsonDocument doc;
                if (deserializeJson(doc, server.arg("plain")) != DeserializationError::Ok)
                {
                  server.send(400, "application/json", "{\"error\":\"invalid JSON body\"}");
                  return;
                }
                if (doc["power"].is<bool>())
                {
                  Lights.SetPower(doc["power"].as<bool>());
                }
                if (doc["brightness"].is<unsigned>())
                {
                  int value = doc["brightness"].as<int>();
                  Lights.SetBrightness(static_cast<uint8_t>(constrain(value, 0, 255)));
                }
                if (doc["theme"].is<const char*>())
                {
                  if (!Lights.ApplyTheme(doc["theme"].as<const char*>()))
                  {
                    server.send(404, "application/json", "{\"error\":\"unknown theme\"}");
                    return;
                  }
                }
                server.send(200, "application/json", stateJson());
              });

    server.on("/api/pixels", HTTP_GET,
              []()
              {
                JsonDocument doc;
                JsonArray pixels = doc["pixels"].to<JsonArray>();
                for (const auto& c : Lights.Pixels())
                {
                  pixels.add(rgbToHex(c));
                }
                String out;
                serializeJson(doc, out);
                server.send(200, "application/json", out);
              });

    server.on("/api/themes", HTTP_GET,
              []()
              {
                server.send(200, "application/json", themesJson());
              });

    server.on("/api/themes", HTTP_POST,
              []()
              {
                JsonDocument doc;
                if (deserializeJson(doc, server.arg("plain")) != DeserializationError::Ok)
                {
                  server.send(400, "application/json", "{\"error\":\"invalid JSON body\"}");
                  return;
                }
                const char* name = doc["name"] | "";
                const char* mode = doc["mode"] | "solid";
                std::vector<Color> colors;
                for (JsonVariant hex : doc["colors"].as<JsonArray>())
                {
                  Color c{};
                  if (hexColorToRGB(std::string(hex.as<const char*>() ? hex.as<const char*>() : ""), c))
                  {
                    colors.push_back(c);
                  }
                }
                ThemeMode modeEnum;
                std::string modeString = mode ? mode : "solid";
                if (!ThemeModeFromName(modeString, modeEnum) || name == nullptr || colors.empty())
                {
                  server.send(400, "application/json", "{\"error\":\"need name, valid mode, and at least one #RRGGBB color\"}");
                  return;
                }
                if (!Lights.UpsertCustomTheme(name, modeEnum, colors))
                {
                  server.send(400, "application/json", "{\"error\":\"theme rejected (empty, or name collides with a builtin)\"}");
                  return;
                }
                server.send(200, "application/json", themesJson());
              });

    server.on("/api/themes", HTTP_DELETE,
              []()
              {
                if (!server.hasArg("name"))
                {
                  server.send(400, "application/json", "{\"error\":\"missing ?name=\"}");
                  return;
                }
                String name = server.arg("name");
                if (!Lights.DeleteCustomTheme(name.c_str()))
                {
                  server.send(404, "application/json", "{\"error\":\"no such custom theme\"}");
                  return;
                }
                server.send(200, "application/json", themesJson());
              });

    server.begin();
    isInitialized = true;
  }

  void Poll(void) override
  {
    server.handleClient();
  }

private:
  static String stateJson(void)
  {
    JsonDocument doc;
    doc["power"]      = Lights.Power();
    doc["brightness"] = Lights.Brightness();
    doc["theme"]      = Lights.ActiveTheme();
    doc["mode"]       = ThemeModeName(Lights.ActiveThemeMode());
    JsonArray colors   = doc["colors"].to<JsonArray>();
    for (const auto& c : Lights.ActiveThemeColors())
    {
      colors.add(rgbToHex(c));
    }

    String out;
    serializeJson(doc, out);
    return out;
  }

  static String themesJson(void)
  {
    JsonDocument doc;
    JsonArray themes = doc["themes"].to<JsonArray>();
    const std::string active = Lights.ActiveTheme();
    for (const auto& theme : Lights.Themes())
    {
      JsonObject t   = themes.add<JsonObject>();
      t["name"]      = theme.name;
      t["mode"]      = ThemeModeName(theme.mode);
      t["builtin"]   = theme.builtin;
      t["active"]    = theme.name == active;
      JsonArray cols = t["colors"].to<JsonArray>();
      for (const auto& c : theme.colors)
      {
        cols.add(rgbToHex(c));
      }
    }

    String out;
    serializeJson(doc, out);
    return out;
  }
};
