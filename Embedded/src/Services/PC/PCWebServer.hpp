#pragma once
#include <thread>
#include <string>
#include "Configuration.hpp"
#include "Lights.hpp"
#include "Logger.hpp"
#include "json.hpp"
#include "httplib.h"
#include "RESTServer.hpp"
#include "web.hpp"
#include "iWebServer.hpp"

class PCWebServer : public iWebServer
{
  enum class Status
  {
    Started = 0x0,
    Stopped
  };

public:
  ~PCWebServer()
  {
    Stop();
  }

  void Init(void) override
  {
    server.Get("/",
               [](const Request&)
               {
                 return Response::Content(INDEX_HTML, "text/html");
               });

    server.Get("/style.css",
               [](const Request&)
               {
                 return Response::Content(STYLE_CSS, "text/css");
               });

    server.Get("/script.js",
               [](const Request&)
               {
                 return Response::Content(SCRIPT_JS, "application/javascript");
               });

    server.Get("/api/state",
               [](const Request&)
               {
                 return Response::Json(stateJson().dump());
               });

    server.Post("/api/state",
                [](const Request& request)
                {
                  auto body = nlohmann::json::parse(request.Body(), nullptr, false);
                  if (body.is_discarded())
                  {
                    return Response::Error(400, "{\"error\":\"invalid JSON body\"}");
                  }
                  if (body.contains("power") && body["power"].is_boolean())
                  {
                    Lights.SetPower(body["power"].get<bool>());
                  }
                  if (body.contains("brightness") && body["brightness"].is_number_unsigned())
                  {
                    int value = body["brightness"].get<int>();
                    Lights.SetBrightness(static_cast<uint8_t>(std::max(0, std::min(255, value))));
                  }
                  if (body.contains("theme") && body["theme"].is_string())
                  {
                    if (!Lights.ApplyTheme(body["theme"].get<std::string>()))
                    {
                      return Response::Error(404, "{\"error\":\"unknown theme\"}");
                    }
                  }
                  return Response::Json(stateJson().dump());
                });

    server.Get("/api/pixels",
               [](const Request&)
               {
                 nlohmann::json pixels = nlohmann::json::array();
                 for (const auto& c : Lights.Pixels())
                 {
                   pixels.push_back(rgbToHex(c));
                 }
                 return Response::Json(nlohmann::json{{ "pixels", pixels }}.dump());
               });

    server.Get("/api/themes",
               [](const Request&)
               {
                 return Response::Json(themesJson().dump());
               });

    server.Post("/api/themes",
                [](const Request& request)
                {
                  auto body = nlohmann::json::parse(request.Body(), nullptr, false);
                  if (body.is_discarded())
                  {
                    return Response::Error(400, "{\"error\":\"invalid JSON body\"}");
                  }
                  std::string name = body.value("name", std::string{});
                  std::string mode = body.value("mode", std::string("solid"));
                  std::vector<Color> colors;
                  for (const auto& hexJson : body.value("colors", nlohmann::json::array()))
                  {
                    Color c{};
                    if (hexJson.is_string() && hexColorToRGB(hexJson.get<std::string>(), c))
                    {
                      colors.push_back(c);
                    }
                  }
                  ThemeMode modeEnum;
                  if (name.empty() || colors.empty() || !ThemeModeFromName(mode, modeEnum))
                  {
                    return Response::Error(400, "{\"error\":\"need name, valid mode, and at least one #RRGGBB color\"}");
                  }
                  if (!Lights.UpsertCustomTheme(name, modeEnum, colors))
                  {
                    return Response::Error(400, "{\"error\":\"theme rejected (empty, or name collides with a builtin)\"}");
                  }
                  return Response::Json(themesJson().dump());
                });

    server.Delete("/api/themes",
                  [](const Request& request)
                  {
                    const std::string name = request.Param("name");
                    if (name.empty())
                    {
                      return Response::Error(400, "{\"error\":\"missing ?name=\"}");
                    }
                    if (!Lights.DeleteCustomTheme(name))
                    {
                      return Response::Error(404, "{\"error\":\"no such custom theme\"}");
                    }
                    return Response::Json(themesJson().dump());
                  });
    isInitialized = true;
  }

  void Poll(void) override
  {
    if (!IsRunning())
    {
      Start();
    }
  }

  void Start(void)
  {
    if (status == Status::Started)
    {
      logger.logf("REST server already started\n");
      return;
    }
    serverThread = std::thread(
            [&]
            {
              server.Start(Configuration.HTTPPort);
            });
    logger.logf("http://localhost:%hu/\n", Configuration.HTTPPort);
    logger.logf("http://localhost:%hu/api/state\n", Configuration.HTTPPort);
    status = Status::Started;
  }

  bool IsRunning(void)
  {
    return status == Status::Started;
  }

  void Stop(void)
  {
    if (status == Status::Started)
    {
      server.Stop();
      serverThread.join();
      status = Status::Stopped;
    }
  }

private:
  static nlohmann::json stateJson(void)
  {
    nlohmann::json colors = nlohmann::json::array();
    for (const auto& c : Lights.ActiveThemeColors())
    {
      colors.push_back(rgbToHex(c));
    }
    return {
      { "power",      Lights.Power() },
      { "brightness", Lights.Brightness() },
      { "theme",      Lights.ActiveTheme() },
      { "mode",       ThemeModeName(Lights.ActiveThemeMode()) },
      { "colors",     colors },
    };
  }

  static nlohmann::json themesJson(void)
  {
    nlohmann::json themes = nlohmann::json::array();
    const std::string active = Lights.ActiveTheme();
    for (const auto& theme : Lights.Themes())
    {
      nlohmann::json t;
      t["name"]    = theme.name;
      t["mode"]    = ThemeModeName(theme.mode);
      t["builtin"] = theme.builtin;
      t["active"]  = theme.name == active;
      t["colors"]  = nlohmann::json::array();
      for (const auto& c : theme.colors)
      {
        t["colors"].push_back(rgbToHex(c));
      }
      themes.push_back(t);
    }
    return { { "themes", themes } };
  }

  RESTServer server;
  std::thread serverThread;
  Status status = Status::Stopped;
};
