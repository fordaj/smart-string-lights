#pragma once
// Platform-selected theme store: NVS on the ESP32, lights.json on the host.
#include "ThemeEngine.hpp"

#ifdef PLATFORM_ARDUINO
  #include "PreferencesThemeStore.hpp"
using ThemeStoreImpl = PreferencesThemeStore;
#elif defined(PLATFORM_PC)
  #include "FileThemeStore.hpp"
using ThemeStoreImpl = FileThemeStore;
#endif
