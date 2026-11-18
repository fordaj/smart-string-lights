#pragma once
// ThemeEngine — platform-agnostic core of smart-string-lights.
//
// No Arduino, no FastLED, no JSON: themes, render math, and persisted state
// live here so the whole engine is exercisable from the host (`pio test`).
// Platform layers provide an iThemeStore (persistence) and turn the rendered
// pixels into light (FastLED on the ESP32, ANSI blocks on the host).

#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>
#include <memory>
#include <algorithm>
#include <cmath>

struct Color
{
  uint8_t r = 0;
  uint8_t g = 0;
  uint8_t b = 0;

  bool operator==(const Color& other) const
  {
    return r == other.r && g == other.g && b == other.b;
  }
};

inline bool hexColorToRGB(const std::string& hex, Color& out)
{
  auto parse = [](const std::string& s, size_t start) { return std::stoi(s.substr(start, 2), nullptr, 16); };
  if (hex.size() != 7 || hex[0] != '#') {
    return false;
  }
  try
  {
    out.r = static_cast<uint8_t>(parse(hex, 1));
    out.g = static_cast<uint8_t>(parse(hex, 3));
    out.b = static_cast<uint8_t>(parse(hex, 5));
    return true;
  }
  catch (...)
  {
    return false;
  }
}

inline std::string rgbToHex(const Color& c)
{
  char buf[8];
  std::snprintf(buf, sizeof(buf), "#%02X%02X%02X", c.r, c.g, c.b);
  return std::string(buf);
}

enum class ThemeMode : uint8_t
{
  Solid   = 0,
  Gradient = 1,
  Stripes = 2,
};

inline const char* ThemeModeName(ThemeMode mode)
{
  switch (mode)
  {
    case ThemeMode::Solid:
      return "solid";
    case ThemeMode::Gradient:
      return "gradient";
    case ThemeMode::Stripes:
      return "stripes";
    default:
      return "solid";
  }
}

inline bool ThemeModeFromName(const std::string& name, ThemeMode& out)
{
  if (name == "solid") { out = ThemeMode::Solid; return true; }
  if (name == "gradient") { out = ThemeMode::Gradient; return true; }
  if (name == "stripes") { out = ThemeMode::Stripes; return true; }
  return false;
}

struct Theme
{
  std::string name;
  std::vector<Color> colors;
  ThemeMode mode   = ThemeMode::Solid;
  bool builtin     = false;
};

struct LightsState
{
  bool power                = true;
  uint8_t brightness        = 128;
  std::string activeTheme   = "Candy Cane";
  std::vector<Theme> customThemes;
};

// Persistence boundary. Both platform layers implement this over a raw
// LightsState, so the engine never sees a JSON library.
class iThemeStore
{
public:
  virtual ~iThemeStore()                   = default;
  virtual bool Load(LightsState& out)      = 0; // false => nothing stored yet
  virtual void Save(const LightsState& in) = 0;
};

namespace BuiltinThemes
{
  inline Color rgb(uint8_t r, uint8_t g, uint8_t b)
  {
    return Color{ r, g, b };
  }

  inline const std::vector<Theme>& All()
  {
    static const std::vector<Theme> themes = {
      { "Candy Cane",      { rgb(0xD6, 0x29, 0x1F), rgb(0xFF, 0xFF, 0xFF) },                                    ThemeMode::Stripes,  true },
      { "Christmas",       { rgb(0xC8, 0x1E, 0x1E), rgb(0x16, 0x5B, 0x33) },                                    ThemeMode::Stripes,  true },
      { "Holly Berry",     { rgb(0x16, 0x5B, 0x33), rgb(0xC8, 0x1E, 0x1E) },                                    ThemeMode::Gradient, true },
      { "Evergreen",       { rgb(0x16, 0x5B, 0x33) },                                                           ThemeMode::Solid,    true },
      { "Snowfall",        { rgb(0xFF, 0xFF, 0xFF), rgb(0xA9, 0xD7, 0xFF) },                                    ThemeMode::Stripes,  true },
      { "Winter Twilight", { rgb(0x1B, 0x1F, 0x3B), rgb(0x7A, 0x3B, 0x6E), rgb(0xE0, 0xA4, 0x58) },               ThemeMode::Gradient, true },
      { "Warm Candlelight",{ rgb(0xFF, 0x9A, 0x3C), rgb(0xFF, 0xE1, 0xA8) },                                    ThemeMode::Gradient, true },
      { "North Pole",      { rgb(0xC8, 0x1E, 0x1E), rgb(0xFF, 0xFF, 0xFF), rgb(0x16, 0x5B, 0x33) },              ThemeMode::Stripes,  true },
    };
    return themes;
  }
} // namespace BuiltinThemes

class ThemeEngine
{
public:
  void Init(iThemeStore& store)
  {
    store_ = &store;
    LightsState persisted;
    if (store_->Load(persisted))
    {
      state_ = std::move(persisted);
    }
    if (FindTheme(state_.activeTheme) == nullptr)
    {
      state_.activeTheme = BuiltinThemes::All().front().name;
    }
    dirty_ = true;
  }

  // ─── State accessors ─────────────────────────────────────────────────
  bool Power() const
  {
    return state_.power;
  }
  uint8_t Brightness() const
  {
    return state_.brightness;
  }
  const LightsState& Snapshot() const
  {
    return state_;
  }

  std::vector<Theme> Themes() const
  {
    std::vector<Theme> all = BuiltinThemes::All();
    all.insert(all.end(), state_.customThemes.begin(), state_.customThemes.end());
    return all;
  }

  const Theme* ActiveTheme() const
  {
    return FindTheme(state_.activeTheme);
  }

  // ─── Mutations (persist + flag for re-render) ────────────────────────
  bool SetPower(bool on)
  {
    if (state_.power == on)
    {
      return true;
    }
    state_.power = on;
    return Commit();
  }

  bool SetBrightness(uint8_t value)
  {
    state_.brightness = value;
    return Commit();
  }

  bool ApplyTheme(const std::string& name)
  {
    if (FindTheme(name) == nullptr)
    {
      return false;
    }
    state_.activeTheme = name;
    return Commit();
  }

  // Create or replace a custom theme. Names of builtin themes are rejected.
  bool UpsertCustomTheme(const std::string& name, ThemeMode mode, const std::vector<Color>& colors)
  {
    if (name.empty() || colors.empty())
    {
      return false;
    }
    for (const auto& builtin : BuiltinThemes::All())
    {
      if (builtin.name == name)
      {
        return false;
      }
    }
    auto it = std::find_if(state_.customThemes.begin(), state_.customThemes.end(), [&name](const Theme& t) { return t.name == name; });
    Theme theme;
    theme.name     = name;
    theme.colors   = colors;
    theme.mode     = mode;
    theme.builtin  = false;
    if (it == state_.customThemes.end())
    {
      state_.customThemes.push_back(theme);
    }
    else
    {
      *it = theme;
    }
    return Commit();
  }

  // Delete a custom theme; falls back to the first builtin if it was active.
  bool DeleteCustomTheme(const std::string& name)
  {
    auto it = std::find_if(state_.customThemes.begin(), state_.customThemes.end(), [&name](const Theme& t) { return t.name == name; });
    if (it == state_.customThemes.end())
    {
      return false;
    }
    state_.customThemes.erase(it);
    if (state_.activeTheme == name)
    {
      state_.activeTheme = BuiltinThemes::All().front().name;
    }
    return Commit();
  }

  // ─── Rendering ────────────────────────────────────────────────────────
  // Computes one Color per pixel into `out`, sized to `pixelCount`.
  void ComputePixels(size_t pixelCount, std::vector<Color>& out) const
  {
    out.assign(pixelCount, Color{});
    if (!state_.power || pixelCount == 0)
    {
      return;
    }
    const Theme* theme = ActiveTheme();
    if (theme == nullptr || theme->colors.empty())
    {
      return;
    }
    const float brightness = static_cast<float>(state_.brightness) / 255.0f;
    auto scaled             = [brightness](const Color& c)
    {
      return Color{ static_cast<uint8_t>(c.r * brightness), static_cast<uint8_t>(c.g * brightness), static_cast<uint8_t>(c.b * brightness) };
    };

    const auto& colors = theme->colors;
    const size_t stops = colors.size();

    for (size_t i = 0; i < pixelCount; ++i)
    {
      Color pixel{};
      switch (theme->mode)
      {
        case ThemeMode::Solid:
          pixel = colors[0];
          break;
        case ThemeMode::Gradient:
        {
          if (stops == 1 || pixelCount == 1)
          {
            pixel = colors[0];
            break;
          }
          const float t            = static_cast<float>(i) / static_cast<float>(pixelCount - 1) * static_cast<float>(stops - 1);
          const size_t segment     = std::min(static_cast<size_t>(t), stops - 2);
          const float fraction     = t - static_cast<float>(segment);
          const Color& a             = colors[segment];
          const Color& b             = colors[segment + 1];
          pixel.r                  = static_cast<uint8_t>(a.r + (b.r - a.r) * fraction);
          pixel.g                  = static_cast<uint8_t>(a.g + (b.g - a.g) * fraction);
          pixel.b                  = static_cast<uint8_t>(a.b + (b.b - a.b) * fraction);
          break;
        }
        case ThemeMode::Stripes:
          pixel = colors[(i * stops) / pixelCount];
          break;
      }
      out[i] = scaled(pixel);
    }
  }

  bool ConsumeDirty()
  {
    const bool was = dirty_;
    dirty_         = false;
    return was;
  }

  void MarkDirty()
  {
    dirty_ = true;
  }

private:
  const Theme* FindTheme(const std::string& name) const
  {
    for (const auto& theme : BuiltinThemes::All())
    {
      if (theme.name == name)
      {
        return &theme;
      }
    }
    for (const auto& theme : state_.customThemes)
    {
      if (theme.name == name)
      {
        return &theme;
      }
    }
    return nullptr;
  }

  bool Commit()
  {
    dirty_ = true;
    if (store_ != nullptr)
    {
      store_->Save(state_);
    }
    return true;
  }

  LightsState state_;
  iThemeStore* store_ = nullptr;
  bool dirty_         = true;
};
