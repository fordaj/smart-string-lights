#pragma once
// File-backed theme store (host). Persists LightsState as lights.json in the
// working directory.
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "ThemeEngine.hpp"
#include "json.hpp"

class FileThemeStore : public iThemeStore
{
public:
  bool Load(LightsState& out) override
  {
    std::ifstream file("lights.json");
    if (!file.good())
    {
      return false;
    }
    std::stringstream contents;
    contents << file.rdbuf();

    nlohmann::json doc;
    try
    {
      doc = nlohmann::json::parse(contents.str());
    }
    catch (...)
    {
      return false;
    }

    out.power         = doc.value("power", true);
    out.brightness    = doc.value("brightness", uint8_t(128));
    out.activeTheme   = doc.value("activeTheme", std::string("Candy Cane"));
    out.customThemes.clear();
    for (const auto& t : doc.value("customThemes", nlohmann::json::array()))
    {
      Theme theme;
      theme.name    = t.value("name", std::string{});
      std::string modeName = t.value("mode", std::string("solid"));
      ThemeMode mode;
      theme.mode    = ThemeModeFromName(modeName, mode) ? mode : ThemeMode::Solid;
      theme.builtin = false;
      for (const auto& hexJson : t.value("colors", nlohmann::json::array()))
      {
        Color c{};
        if (hexJson.is_string() && hexColorToRGB(hexJson.get<std::string>(), c))
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
    nlohmann::json doc;
    doc["power"]       = in.power;
    doc["brightness"]  = in.brightness;
    doc["activeTheme"] = in.activeTheme;
    nlohmann::json custom = nlohmann::json::array();
    for (const auto& theme : in.customThemes)
    {
      nlohmann::json t;
      t["name"]     = theme.name;
      t["mode"]     = ThemeModeName(theme.mode);
      t["colors"]   = nlohmann::json::array();
      for (const auto& c : theme.colors)
      {
        t["colors"].push_back(rgbToHex(c));
      }
      custom.push_back(t);
    }
    doc["customThemes"] = custom;

    std::ofstream file("lights.json");
    file << doc.dump(2);
  }
};
