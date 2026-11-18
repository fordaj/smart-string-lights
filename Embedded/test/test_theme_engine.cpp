// Host-side unit tests for the platform-agnostic theme engine.
// Run with: pio test -e host
#include <unity.h>
#include "ThemeEngine.hpp"

#include <vector>

namespace
{
  struct MockStore : iThemeStore
  {
    LightsState saved;
    bool hasSaved = false;

    bool Load(LightsState& out) override
    {
      if (!hasSaved)
      {
        return false;
      }
      out = saved;
      return true;
    }

    void Save(const LightsState& in) override
    {
      saved     = in;
      hasSaved  = true;
    }
  };

  std::vector<Color> hexList(std::initializer_list<const char*> hexes)
  {
    std::vector<Color> out;
    for (const char* hex : hexes)
    {
      Color c{};
      hexColorToRGB(hex, c);
      out.push_back(c);
    }
    return out;
  }

  std::vector<Color> render(const ThemeEngine& engine, size_t pixelCount)
  {
    std::vector<Color> pixels;
    engine.ComputePixels(pixelCount, pixels);
    return pixels;
  }
} // namespace

void test_builtin_themes_are_present(void)
{
  MockStore store;
  ThemeEngine engine;
  engine.Init(store);

  auto themes = engine.Themes();
  TEST_ASSERT_GREATER_OR_EQUAL(6, themes.size());
  bool foundCandyCane = false;
  for (const auto& t : themes)
  {
    if (t.name == "Candy Cane")
    {
      foundCandyCane = true;
    }
  }
  TEST_ASSERT_TRUE(foundCandyCane);
}

void test_default_state_is_candy_cane_full_brightness_off_power_not(void)
{
  MockStore store;
  ThemeEngine engine;
  engine.Init(store);

  TEST_ASSERT_TRUE(engine.Power());
  TEST_ASSERT_EQUAL_UINT8(128, engine.Brightness());
  TEST_ASSERT_EQUAL_STRING("Candy Cane", engine.ActiveTheme()->name.c_str());
}

void test_stripes_split_pixels_evenly(void)
{
  MockStore store;
  ThemeEngine engine;
  engine.Init(store);
  engine.ApplyTheme("Candy Cane");
  engine.SetBrightness(255);

  auto pixels = render(engine, 10);
  TEST_ASSERT_EQUAL_UINT32(0xD6291F, (uint32_t(pixels[0].r) << 16) | (uint32_t(pixels[0].g) << 8) | pixels[0].b);
  TEST_ASSERT_EQUAL_UINT32(0xD6291F, (uint32_t(pixels[4].r) << 16) | (uint32_t(pixels[4].g) << 8) | pixels[4].b);
  TEST_ASSERT_EQUAL_UINT32(0xFFFFFF, (uint32_t(pixels[5].r) << 16) | (uint32_t(pixels[5].g) << 8) | pixels[5].b);
  TEST_ASSERT_EQUAL_UINT32(0xFFFFFF, (uint32_t(pixels[9].r) << 16) | (uint32_t(pixels[9].g) << 8) | pixels[9].b);
}

void test_gradient_endpoints_and_midpoint(void)
{
  MockStore store;
  ThemeEngine engine;
  engine.Init(store);
  engine.SetBrightness(255);
  engine.ApplyTheme("Winter Twilight"); // #1B1F3B -> #7A3B6E -> #E0A458

  auto pixels = render(engine, 11);
  TEST_ASSERT_EQUAL_UINT8(0xE0, pixels[10].r);
  TEST_ASSERT_EQUAL_UINT8(0x1B, pixels[0].r);
  TEST_ASSERT_EQUAL_UINT8(0x3B, pixels[0].b);
  // Midpoint of a 3-stop gradient lands exactly on the middle stop
  TEST_ASSERT_EQUAL_UINT8(0x7A, pixels[5].r);
  TEST_ASSERT_EQUAL_UINT8(0x3B, pixels[5].g);
  TEST_ASSERT_EQUAL_UINT8(0x6E, pixels[5].b);
}

void test_solid_fills_every_pixel(void)
{
  MockStore store;
  ThemeEngine engine;
  engine.Init(store);
  engine.SetBrightness(255);
  engine.ApplyTheme("Evergreen");

  auto pixels = render(engine, 25);
  for (const auto& p : pixels)
  {
    TEST_ASSERT_EQUAL_UINT8(0x16, p.r);
    TEST_ASSERT_EQUAL_UINT8(0x5B, p.g);
    TEST_ASSERT_EQUAL_UINT8(0x33, p.b);
  }
}

void test_power_off_renders_black(void)
{
  MockStore store;
  ThemeEngine engine;
  engine.Init(store);
  engine.SetBrightness(255);
  engine.SetPower(false);

  auto pixels = render(engine, 10);
  for (const auto& p : pixels)
  {
    TEST_ASSERT_EQUAL_UINT8(0, p.r);
    TEST_ASSERT_EQUAL_UINT8(0, p.g);
    TEST_ASSERT_EQUAL_UINT8(0, p.b);
  }
}

void test_brightness_scales_pixels(void)
{
  MockStore store;
  ThemeEngine engine;
  engine.Init(store);
  engine.ApplyTheme("Candy Cane");
  engine.SetBrightness(0);
  for (const auto& p : render(engine, 10))
  {
    TEST_ASSERT_EQUAL_UINT8(0, p.r);
    TEST_ASSERT_EQUAL_UINT8(0, p.g);
    TEST_ASSERT_EQUAL_UINT8(0, p.b);
  }

  engine.SetBrightness(128);
  auto dim = render(engine, 10);
  engine.SetBrightness(255);
  auto full = render(engine, 10);
  TEST_ASSERT_TRUE(dim[0].r > 0 && dim[0].r < full[0].r);
}

void test_custom_theme_round_trip_through_store(void)
{
  MockStore store;
  {
    ThemeEngine engine;
    engine.Init(store);
    TEST_ASSERT_TRUE(engine.UpsertCustomTheme("Mi Tema", ThemeMode::Stripes, hexList({ "#FF0000", "#0000FF" })));
    TEST_ASSERT_TRUE(engine.ApplyTheme("Mi Tema"));
    TEST_ASSERT_TRUE(engine.SetBrightness(200));
  }
  // Fresh engine over the same store must restore persisted state
  ThemeEngine reloaded;
  reloaded.Init(store);
  TEST_ASSERT_TRUE(reloaded.Power());
  TEST_ASSERT_EQUAL_UINT8(200, reloaded.Brightness());
  TEST_ASSERT_EQUAL_STRING("Mi Tema", reloaded.ActiveTheme()->name.c_str());
  bool foundCustom = false;
  for (const auto& t : reloaded.Themes())
  {
    if (t.name == "Mi Tema")
    {
      foundCustom = true;
      TEST_ASSERT_FALSE(t.builtin);
      TEST_ASSERT_EQUAL_UINT32(2, t.colors.size());
    }
  }
  TEST_ASSERT_TRUE(foundCustom);
}

void test_custom_theme_cannot_shadow_a_builtin(void)
{
  MockStore store;
  ThemeEngine engine;
  engine.Init(store);
  TEST_ASSERT_FALSE(engine.UpsertCustomTheme("Candy Cane", ThemeMode::Solid, hexList({ "#000000" })));
  TEST_ASSERT_FALSE(engine.UpsertCustomTheme("", ThemeMode::Solid, hexList({ "#000000" })));
  TEST_ASSERT_FALSE(engine.UpsertCustomTheme("Empty", ThemeMode::Solid, {}));
}

void test_deleting_active_custom_theme_falls_back_to_builtin(void)
{
  MockStore store;
  ThemeEngine engine;
  engine.Init(store);
  engine.UpsertCustomTheme("Temporary", ThemeMode::Solid, hexList({ "#123456" }));
  engine.ApplyTheme("Temporary");

  TEST_ASSERT_TRUE(engine.DeleteCustomTheme("Temporary"));
  TEST_ASSERT_TRUE(engine.ActiveTheme()->builtin);
  TEST_ASSERT_FALSE(engine.DeleteCustomTheme("Temporary")); // already gone
}

void test_hex_color_parsing(void)
{
  Color c{};
  TEST_ASSERT_TRUE(hexColorToRGB("#D6291F", c));
  TEST_ASSERT_EQUAL_UINT8(0xD6, c.r);
  TEST_ASSERT_EQUAL_UINT8(0x29, c.g);
  TEST_ASSERT_EQUAL_UINT8(0x1F, c.b);
  TEST_ASSERT_FALSE(hexColorToRGB("#FFF", c));
  TEST_ASSERT_FALSE(hexColorToRGB("FFFFFF", c));
  TEST_ASSERT_FALSE(hexColorToRGB("#GGGGGG", c));
  TEST_ASSERT_EQUAL_STRING("#D6291F", rgbToHex(c).c_str());
}

void setUp(void)
{
}

void tearDown(void)
{
}

int main(int, char**)
{
  UNITY_BEGIN();
  RUN_TEST(test_builtin_themes_are_present);
  RUN_TEST(test_default_state_is_candy_cane_full_brightness_off_power_not);
  RUN_TEST(test_stripes_split_pixels_evenly);
  RUN_TEST(test_gradient_endpoints_and_midpoint);
  RUN_TEST(test_solid_fills_every_pixel);
  RUN_TEST(test_power_off_renders_black);
  RUN_TEST(test_brightness_scales_pixels);
  RUN_TEST(test_custom_theme_round_trip_through_store);
  RUN_TEST(test_custom_theme_cannot_shadow_a_builtin);
  RUN_TEST(test_deleting_active_custom_theme_falls_back_to_builtin);
  RUN_TEST(test_hex_color_parsing);
  return UNITY_END();
}
