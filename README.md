# smart-string-lights
RGB holiday themed string lights with smart home support.

Built on the same toolchain as [AirFrame](https://github.com/fordaj/AirFrame) (which this repo will eventually share code with via submodules). Targets the same ESP32-S3 dev board.

## Building

Requires [uv](https://docs.astral.sh/uv/). The PlatformIO toolchain runs out of a
project venv (`uv sync`), not a global install:

```bash
Embedded/build.sh -m ESP32_S3   # build the ESP32-S3 firmware
Embedded/flash.sh -m ESP32_S3   # build + flash over USB
Embedded/run.sh   -m ESP32_S3   # serial monitor at 115200
```

`Embedded/detect.sh` checks whether an ESP32-S3 is connected. The build
materialises `.version/Version.hpp` from git history via `scripts/version.sh`;
see `scripts/README.md` for the environment cascade and `tools/README.md` for
the bundled helper binaries.
