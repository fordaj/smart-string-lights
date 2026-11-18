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

# ----- Set debug/release ----- #
if [[ "${DEBUG:-}" == "true" ]]; then
  PIO_OPERATION=debug
else
  PIO_OPERATION=run
fi

# ----- Pre Build ----- #
if [[ "${CLEAN:-}" == "true" ]]; then
  source "$CLEAN_SH"
fi
source "$GENERATE_WEB_HPP_SH"
# ----- Assemble build args ----- #
log INFO "Building mode: $PROJECT_MODE (pio operation: $PIO_OPERATION)" &
LOG_PIDS+=($!)
case "$PROJECT_MODE" in
ESP32_S3 | host | ESP32_S3_EMULATED) run_pio_cmd "$PIO_EXE" "$PIO_OPERATION" --environment "$PROJECT_MODE" ;;
all) run_pio_cmd "$PIO_EXE" "$PIO_OPERATION" ;;
*)
  log ERROR "Unsupported mode: $PROJECT_MODE" &
  wait "${LOG_PIDS[@]}"
  exit 1
  ;;
esac

# ----- Post Build ----- #
if [[ "${FLASH:-}" == "true" ]]; then
  source "$FLASH_SH"
fi

wait "${LOG_PIDS[@]}"
