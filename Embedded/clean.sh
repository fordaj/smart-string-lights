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

# ----- Assemble build args ----- #
log INFO "Cleaning mode: $PROJECT_MODE" &
LOG_PIDS+=($!)
case "$PROJECT_MODE" in
ESP32_S3 | ESP32_S3_EMULATED)
  run_pio_cmd "$PIO_EXE" run --environment "$PROJECT_MODE" --target clean
  ;;
all)
  run_pio_cmd "$PIO_EXE" run --target clean
  ;;
*)
  log ERROR "Unsupported mode: $PROJECT_MODE" &
  LOG_PIDS+=($!)
  wait "${LOG_PIDS[@]}"
  exit 1
  ;;
esac

wait "${LOG_PIDS[@]}"
