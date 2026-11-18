#!/usr/bin/env bash
# Embedded/helpers.sh — shared library for the embedded task scripts.
#
#   source Embedded/helpers.sh
#
# Sources scripts/entry.sh (env cascade + log()), re-materialises
# .version/Version.hpp/Version.txt via scripts/version.sh --write, parses
# the shared command-line flags (-m/--mode, -l/--log-level, -h/--help,
# -f/--flash, -d/--debug, --clean, --rebuild, --regen-all, --skip-zip),
# and provides the PlatformIO wrappers (setup_penv/run_pio_cmd) and the
# PID helpers. Not an entry point itself; it is sourced by Embedded/*.sh.

# Resolve the repository root from this file's location so the scripts
# work from any working directory.
REPO_ROOT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$REPO_ROOT_DIR"

# shellcheck disable=SC1091
source "$REPO_ROOT_DIR/scripts/entry.sh"

# ─── Materialise the version for the C++ build (like the old
# update_version_dir.sh, but through versioner), and export VERSION/TAG ──
if git -C "$REPO_ROOT_DIR" rev-parse --is-inside-work-tree >/dev/null 2>&1; then
  if ! eval "$("$REPO_ROOT_DIR/scripts/version.sh" --format shell --write)"; then
    log WARNING "failed to compute/write ${VERSION_DIR:-.version}; the build may use a stale Version.hpp"
  fi
fi

# --- Usage  --- #
usage() {
  cat <<EOF
Usage: $(basename "$0") [OPTIONS]

Options:
  -m, --mode     PIO mode to build in {ESP32_S3, ESP32_S3_EMULATED, all}
  -l, --log-level  Log level {TRACE, DEBUG, INFO, WARNING, ERROR, CRITICAL} (default: INFO)
  -f, --flash    Flash after building (where supported)
  -d, --debug    Build with pio debug instead of pio run
  --clean        Clean before building
  --rebuild      Rebuild before running
  --regen-all    Regenerate everything the script is able to regenerate
  --skip-zip     Skip zip creation (release scripts)
  -h, --help     Show this help message and exit.
EOF
}

# --- Parse Args  --- #
# Consumes the shared flags and leaves the rest in "$@".
PASSTHROUGH_ARGS=()
while [[ $# -gt 0 ]]; do
  case "$1" in
  -h | --help)
    export HELP=true
    shift
    ;;
  -m | --mode)
    if [[ $# -lt 2 ]]; then
      echo "error: -m|--mode requires an argument" >&2
      exit 1
    fi
    export PROJECT_MODE="$2"
    shift 2
    ;;
  -f | --flash)
    export FLASH=true
    shift
    ;;
  -d | --debug)
    export DEBUG=true
    shift
    ;;
  -l | --log-level)
    if [[ $# -lt 2 ]]; then
      echo "error: --log-level requires an argument" >&2
      exit 1
    fi
    LOG_LEVEL="$(printf '%s' "$2" | tr '[:lower:]' '[:upper:]')"
    export LOG_LEVEL
    LOG_LEVEL_NAME="$(log_level_name "$LOG_LEVEL")"
    export LOG_LEVEL_NAME
    shift 2
    ;;
  --clean)
    export CLEAN=true
    shift
    ;;
  --rebuild)
    export REBUILD=true
    shift
    ;;
  --regen-all)
    export REGEN_ALL=true
    shift
    ;;
  --skip-zip)
    export SKIP_ZIP=true
    shift
    ;;
  *)
    PASSTHROUGH_ARGS+=("$1")
    shift
    ;;
  esac
done

if [[ "${HELP:-}" == "true" ]]; then
  usage
  exit 0
fi

set -- "${PASSTHROUGH_ARGS[@]}"

# --- Logging bookkeeping (scripts background log calls and await them) ---
LOG_PIDS=()

# --- Embedded toolchain helpers --- #

# Create or update the project venv from pyproject.toml/uv.lock.
setup_penv() {
  if [ -d "$VENV" ]; then
    return 0
  fi
  if command -v uv >/dev/null 2>&1; then
    (cd "$REPO_ROOT_DIR" && uv sync --locked)
    return $?
  fi
  log WARNING "uv not found on PATH; falling back to python -m venv"
  local python_bin
  python_bin="$(command -v python3 || command -v python)"
  [[ -n "$python_bin" ]] || {
    echo "error: neither uv nor python is available to create $VENV" >&2
    return 1
  }
  "$python_bin" -m venv "$VENV"
  "$PYTHON_EXE" -m pip install --upgrade pip
  "$PYTHON_EXE" -m pip install platformio==6.1.19 esptool==5.3.1 # keep in sync with pyproject.toml
}

# Wrap pio-style commands and ensure they run in the environment they expect.
run_pio_cmd() {
  ( # Subshell for the venv-provided toolchain
    if [ ! -x "$PIO_EXE" ]; then
      setup_penv || exit 1
    fi
    local exe="$1"
    shift
    # NOTE: exec replaces this subshell with the target so callers that
    # track RUN_PID (Embedded/run.sh, Embedded/test.sh) signal the real
    # process, not an intermediate bash - otherwise the orphan keeps the
    # stdout pipe open and test.sh never terminates.
    case "$(uname -s):$(uname -m)" in
    Darwin:x86_64) exec "$exe" "$@" ;;
    MINGW*:x86_64 | MSYS*:x86_64 | CYGWIN*:x86_64)
      # QUIRK: Some of the tools invoked use forbidden chars; setting these vars fixes it.
      export PYTHONUTF8=1
      export PYTHONIOENCODING=utf-8
      export LC_ALL=C.UTF-8
      export PLATFORMIO_BUILD_FLAGS="-DEMULATOR"
      # QUIRK: The ESP toolchain, on install, fails if these environment vars are set. Running through PowerShell and stubbing them out makes it happy.
      PIO_EXE="$exe" exec powershell.exe \
        -NoProfile \
        -ExecutionPolicy Bypass \
        -File "$RUN_PIO_CMD_PS1" \
        "$@"
      ;;
    Darwin:arm64 | Linux:x86_64 | Linux:aarch64 | *)
      echo "Unsupported platform: $(uname -s) $(uname -m)"
      return 1
      ;;
    esac
  )
}

kill_pid_if_running() {
  local pid="$1"
  if kill -0 "$pid" 2>/dev/null; then
    log TRACE "PID $pid running, killing PID"
    kill "$pid"
  else
    log TRACE "PID $pid not running"
  fi
}

wait_on_pid_if_running() {
  local pid="$1"
  if kill -0 "$pid" 2>/dev/null; then
    log TRACE "PID $pid running, waiting for PID"
    wait "$pid"
  else
    log TRACE "PID $pid not running"
  fi
}
