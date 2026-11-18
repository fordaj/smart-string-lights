# smart-string-lights
RGB holiday themed string lights with smart home support.

Built on the same toolchain as [AirFrame](https://github.com/fordaj/AirFrame) (which this repo will eventually share code with via submodules — see [SUBMODULES.md](SUBMODULES.md)). Targets the same ESP32-S3 dev board.

## Hardware

- ESP32-S3-DevKitC-1 (N16)
- WS2811 strip (5 V or 12 V, 800 kHz protocol, GRB order)
- Data line → **GPIO 5** by default (override at build time with `-DLED_DATA_PIN=<n>` or edit `Embedded/src/Application/Configuration.hpp`), through a ~330 Ω series resistor near the strip; power the strip from its own supply and share grounds. A 1000 µF cap across the strip's supply rails is good practice.
- Pixel count defaults to **50** (`-DLED_COUNT=<n>` or `Configuration.hpp`).

## Building

Requires [uv](https://docs.astral.sh/uv/). The PlatformIO toolchain runs out of a
project venv (`uv sync`), not a global install:

```bash
Embedded/build.sh -m ESP32_S3   # build the ESP32-S3 firmware
Embedded/build.sh -m host       # build the host (PC) version
Embedded/flash.sh  -m ESP32_S3  # build + flash over USB
Embedded/test.sh                # host-side unit tests (theme engine)
```

`Embedded/detect.sh` checks whether an ESP32-S3 is connected. The build
materialises `.version/Version.hpp` from git history via `scripts/version.sh`;
see `scripts/README.md` for the environment cascade and `tools/README.md` for
the bundled helper binaries.

### Host (no hardware) development loop

`-m host` builds the full stack for your PC. The web UI is served at
http://localhost:8080/ and the strip is rendered as truecolor ANSI blocks in
the terminal (note: on some Windows machines the UCRT64 runtime must precede
the legacy mingw64 binaries — `PATH=/c/msys64/ucrt64/bin:$PATH .pio/build/host/program.exe`).

- `GET /api/state` — power, brightness, active theme
- `POST /api/state` — `{"power": bool, "brightness": 0-255, "theme": "name"}` (any subset)
- `GET /api/themes` — builtin + custom themes
- `POST /api/themes` — upsert a custom theme `{"name", "mode": "solid|gradient|stripes", "colors": ["#RRGGBB", ...]}`
- `DELETE /api/themes?name=...` — delete a custom theme
- `GET /api/pixels` — the currently rendered frame as a JSON array of `#RRGGBB` (diagnostics)

Custom themes and the current state persist to `lights.json` on the host and to
NVS on the ESP32. Built-in holiday themes: Candy Cane, Christmas, Holly Berry,
Evergreen, Snowfall, Winter Twilight, Warm Candlelight, North Pole.

### On-device EMULATED mode (real hardware, no LEDs attached)

`pio run -e ESP32_S3_EMULATED` / `Embedded/flash.sh -m ESP32_S3_EMULATED` builds
the firmware with `EMULATED` defined. The FastLED driver is swapped for a stub
that dumps every rendered frame to `Serial` as run-length-encoded hex, and no
GPIO is driven, so themes can be verified on a bare board:

```bash
Embedded/flash.sh -m ESP32_S3_EMULATED
.pio/.../pio device monitor   # or Embedded/run.sh -m ESP32_S3_EMULATED
```

The device still joins WiFi, serves the same UI and API on port 80, persists
state to NVS, and answers `/api/pixels` — everything except actually lighting
pixels. Note that opening/closing the serial monitor hardware-resets the
board; give it ~8–10 s to re-associate with WiFi before curling it.
