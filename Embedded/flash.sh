#!/usr/bin/env bash
set -euo pipefail

# --- Source Scripts --- #
(return 0 2>/dev/null) && sourced=true || sourced=false
if ! "${sourced}"; then
  source "$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)/helpers.sh"
fi

# ----- Validate Inputs ----- #
if [[ -z "${PROJECT_MODE:-}" ]]; then
  echo "Error: mode required" >&2
  usage
  exit 1
fi

# ----- Pre Flash Step ----- #
# No web assets yet; once Embedded/web/ exists, stage it into $PIO_DATA_DIR here.
PIO_DATA_DIR="${PIO_DATA_DIR:-data}"
# ----- Build and Flash ----- #
log INFO "Flashing mode: $PROJECT_MODE" &
LOG_PIDS+=($!)
case "$PROJECT_MODE" in
ESP32_S3 | ESP32_S3_EMULATED)
  run_pio_cmd "$PIO_EXE" run --environment "$PROJECT_MODE" --target upload
  if [[ -d "$PIO_DATA_DIR" ]]; then
    run_pio_cmd "$PIO_EXE" run -e "$PROJECT_MODE" -t uploadfs
  else
    log INFO "no $PIO_DATA_DIR directory; skipping filesystem upload" &
  fi
  ;;
host | all | *)
  log ERROR "Flash not supported for mode: $PROJECT_MODE" &
  LOG_PIDS+=($!)
  exit 1
  ;;
esac

# ----- Post Flash Step ----- #
