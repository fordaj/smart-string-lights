# Submodule proposal: sharing code with AirFrame

smart-string-lights was bootstrapped by copying AirFrame's tooling and embedded
skeleton, and a good chunk of application code still matches AirFrame's layout
file-for-file. This document proposes how to make that "one day, in the form of
submodules" concrete. Status: **proposal only** — nothing is extracted yet, and
the proposed repo names/visibility are decisions for the owner.

## Why now, why not yet

- The two projects already share the *toolchain* (this directory's `tools/`,
  `scripts/`, `platformio.ini`, `pyproject.toml`/`uv.lock`) by deliberate
  copy, so everyone knows the pattern works.
- Submodules pay off when the shared code is stable enough that cross-repo
  friction (version pinning, detached heads) is cheaper than drift. The
  contested ground today is small: roughly the Units, scheduler, logger,
  platform shim, and the WiFi/web-server plumbing.
- Extraction is mechanical once names are agreed: the shared files were copied
  with their AirFrame paths preserved, so `git subtree split` (or a plain copy)
  can seed each new repo.

## Proposed submodule split

### 1. `fordaj/embedded-core` → mounted at `Embedded/shared-core` (or `shared/core`)

Everything that is platform-selectable but application-agnostic. No Arduino,
no FastLED, no WiFi. Host-buildable with zero external dependencies.

| Path today (same in AirFrame)      | Notes                                           |
| ---------------------------------- | ----------------------------------------------- |
| `Embedded/src/Units/BaseUnits.hpp` | Foundational strong-units template              |
| `Embedded/src/Units/Time.hpp`      |                                                 |
| `Embedded/src/Units/Frequency.hpp` |                                                 |
| `Embedded/src/Units/Percentage.hpp`|                                                 |
| `Embedded/src/Units/Units.hpp`     | **Trimmed in MSL** to the three above           |
| `Embedded/src/Infrastructure/iScheduler.hpp` (Task, Platform::Now/Idle) | |
| `Embedded/src/Infrastructure/iResetReason.hpp` | |
| `Embedded/src/Infrastructure/Arduino/Scheduler.hpp` | ms tick from `millis()`            |
| `Embedded/src/Infrastructure/PC/Scheduler.hpp`      | steady_clock ms                      |
| `Embedded/src/Infrastructure/PC/ResetReason.hpp`    | no-op on host                        |
| `Embedded/src/Devices/Logger.hpp`  | abstract `Logger`                               |
| `Embedded/src/Platform/Platform.hpp` | `PLATFORM_ARDUINO` / `PLATFORM_PC` selection  |
| `Embedded/src/Platform/Arduino.hpp` | MSL-trimmed: no `<Wire.h>`, no dead I2C code — **this diff is the extraction delta** |
| `Embedded/src/Platform/PC.hpp`     |                                                 |

AirFrame keeps: every other Units file (its sensors use them), the I2C util.
MSL keeps: nothing of the above — all of it moves.

### 2. `fordaj/embedded-services-esp32` → mounted at `Embedded/shared-esp32` (or `shared/esp32`)

ESP32-specific plumbing. Arduino framework + WiFi dependencies.

| Path today (same in AirFrame)               | Notes                                  |
| ------------------------------------------- | -------------------------------------- |
| `Embedded/src/Services/iServices.hpp`       |                                        |
| `Embedded/src/Services/iWebServer.hpp`      |                                        |
| `Embedded/src/Services/iRestServer.hpp`     | MSL added `Delete()` + `Response::Error()` — **live improvements to upstream** |
| `Embedded/src/Services/PC/RESTServer.hpp`   | hosts the PC httplib wrapper           |
| `Embedded/src/Services/PC/PCWebServer.hpp`  | MSL rewrote the routes; the *threading/skeleton* is shared |
| `Embedded/src/Services/PC/PCServices.hpp`   |                                        |
| `Embedded/src/Services/Services.hpp`        | selection skeleton (no HomeSpan in MSL)|
| `Embedded/src/Services/Arduino/ESP32Services.hpp` | MSL dropped I2C/HomeSpan               |
| `Embedded/src/Services/Arduino/ESP32WiFi.hpp`     | MSL renamed hostname prefix (`SmartLights-%06llX`) — parameterize `Configuration::AppHostName` at extraction |
| `Embedded/src/Services/Arduino/ESP32ResetReason.hpp` |                                  |
| `Embedded/lib/httplib.h`, `Embedded/lib/json.hpp` | vendored third-party; ship as part of this submodule |
| `Embedded/scripts/host.py`                  | Windows LIBS=ws2_32 quirk                |

### Not shared (stays per-repo)

| Stays in AirFrame             | Stays in smart-string-lights            |
| ----------------------------- | ---------------------------------------- |
| `Application/Configuration.*` | `Application/Configuration.hpp`          |
| `Application/Parameters.hpp`  | `Application/ThemeEngine.hpp` and friends (RGB/Color themes, stores, Lights singleton) |
| sensor devices, peripherals   | `Peripherals/*` (WS2811Strip, EmulatedStrip, AnsiStrip, iLEDStrip) |
| `Application/Tasks.hpp` (sensor polling) | `main.cpp` task table              |
| `Embedded/test/protocol.py` (hardware UART tests) | `Embedded/test/test_theme_engine.cpp` (host unit tests) |
| `web/` (sensor dashboard)     | `web/` (theme UI) — but the **web-asset pipeline** (`Embedded/web/{index,style,script}` → `generate_web_hpp.sh` → `web.hpp`, embedded on both platforms) is a shared convention worth encoding in `embedded-core` |

### Deliberately not a submodule: `tools/` and `scripts/`

Already copied by design. The pinned helper binaries are versioned by
directory and consumed via the env cascade; making them a submodule buys
nothing over the current copy-and-pin approach and would complicate checkouts
for no benefit. Revisit if a third repo starts copying them.

## What needs to happen before extraction

1. **Decide repo names, org, and visibility** (`embedded-core` /
   `embedded-services-esp32` under `fordaj`? private or public?).
2. **Parameterize app identity in `ESP32WiFi.hpp`** (hostname prefix) so the
   shared file carries no per-app strings.
3. **Agree the `Response::Error` / `Delete` and Units-trim diffs** — they are
   improvements AirFrame hasn't merged yet (MSL extended `iRESTServer.hpp` and
   `RESTServer.hpp`; AirFrame still has the smaller surface).
4. **License check**: both repos are MIT (or pick one) — submodule LICENSE
   situation needs to be sane before making anything public.
5. **AIRFRAME build impact**: AirFrame starts including `shared/core` and
   `shared/esp32` instead of `src/Units`, `src/Infrastructure`, etc.;
   `-I` flags in `platformio.ini` change accordingly.

## Extraction checklist (when approved)

- [ ] Create both repos; seed with file-for-file copies at matching tags.
- [ ] AirFrame: add submodules, delete copied files, update `platformio.ini`
      include paths, build all three envs, land on main.
- [ ] MSL: flip `-I` flags to the submodule paths, delete copied files,
      rebuild host + ESP32_S3, run `pio test -e host`.
- [ ] Pin both repos in `.gitmodules` with absolute URLs; document in READMEs.
- [ ] Add CI to MSL that clones with `--recursive`.

## Naming/visibility questions for the owner

- `embedded-core` vs `airframe-core` vs `fordaj-embedded`?
- Public or private? (both are effectively source-available today via fordaj)
- One repo instead of two (`embedded` with `core/` + `esp32/` dirs, two
  submodules not needed — simpler, coarser)? Listed here because it is the
  obvious counterproposal; the two-repo split keeps the host build free of
  Arduino headers, which the unit tests rely on.
