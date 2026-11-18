#pragma once
// Lights — owns the theme engine, the strip peripheral, and the persisted
// state. Services (web servers) call into this; the scheduler polls it to
// re-render only when something changed.
#include <vector>
#include <string>

#include "ThemeEngine.hpp"
#include "ThemeStore.hpp"
#include "Peripherals.hpp"

class LightsSingleton
{
public:
  static LightsSingleton& GetLightsSingleton(void)
  {
    static LightsSingleton singleton;
    return singleton;
  }

private:
  LightsSingleton(void)                             = default;
  ~LightsSingleton(void)                            = default;
  LightsSingleton(const LightsSingleton&)           = delete;
  LightsSingleton& operator=(const LightsSingleton&) = delete;

public:
  void Init(void)
  {
    engine_.Init(store_);
    RenderIfDirty(); // paint the strip on boot
  }

  void Poll(void)
  {
    RenderIfDirty();
  }

  // ─── API surface used by the web servers ──────────────────────────────
  bool Power(void) const
  {
    return engine_.Power();
  }
  uint8_t Brightness(void) const
  {
    return engine_.Brightness();
  }
  std::string ActiveTheme(void) const
  {
    auto* theme = engine_.ActiveTheme();
    return theme != nullptr ? theme->name : std::string{};
  }
  ThemeMode ActiveThemeMode(void) const
  {
    auto* theme = engine_.ActiveTheme();
    return theme != nullptr ? theme->mode : ThemeMode::Solid;
  }
  std::vector<Color> ActiveThemeColors(void) const
  {
    auto* theme = engine_.ActiveTheme();
    return theme != nullptr ? theme->colors : std::vector<Color>{};
  }

  bool SetPower(bool on)
  {
    return engine_.SetPower(on);
  }
  bool SetBrightness(uint8_t value)
  {
    return engine_.SetBrightness(value);
  }
  bool ApplyTheme(const std::string& name)
  {
    return engine_.ApplyTheme(name);
  }
  bool UpsertCustomTheme(const std::string& name, ThemeMode mode, const std::vector<Color>& colors)
  {
    return engine_.UpsertCustomTheme(name, mode, colors);
  }
  bool DeleteCustomTheme(const std::string& name)
  {
    return engine_.DeleteCustomTheme(name);
  }

  std::vector<Theme> Themes(void) const
  {
    return engine_.Themes();
  }

  // Current rendered frame (applies power/brightness/theme). Cheap for
  // strip-sized pixel counts; meant for diagnostics (/api/pixels).
  std::vector<Color> Pixels(void)
  {
    std::vector<Color> out;
    engine_.ComputePixels(Peripherals.GetStrip().PixelCount(), out);
    return out;
  }

private:
  void RenderIfDirty(void)
  {
    if (!engine_.ConsumeDirty())
    {
      return;
    }
    auto& strip = Peripherals.GetStrip();
    engine_.ComputePixels(strip.PixelCount(), pixels_);
    strip.Render(pixels_.data(), pixels_.size());
  }

  ThemeStoreImpl store_;
  ThemeEngine engine_;
  std::vector<Color> pixels_;
};

static LightsSingleton& Lights = LightsSingleton::GetLightsSingleton();
