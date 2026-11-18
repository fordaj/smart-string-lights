#!/usr/bin/env bash
set -euo pipefail

# --- Source Scripts --- #
(return 0 2>/dev/null) && sourced=true || sourced=false
if ! "${sourced}"; then
  source "$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)/helpers.sh"
fi

# --- Args  --- #
while [[ $# -gt 0 ]]; do
  case "$1" in
  --run-in-background)
    RUN_IN_BACKGROUND=true
    shift
    ;;
  esac
done

# ----- Validate Inputs ----- #
if [[ -z "${PROJECT_MODE:-}" ]]; then
  echo "Error: mode required" >&2
  usage
  exit 1
fi

# ----- Pre Run ----- #
if [[ "${REBUILD:-}" == "true" ]]; then
  source "$BUILD_SH"
fi

# ----- Run ----- #
log INFO "Running in mode: $PROJECT_MODE" &
case "$PROJECT_MODE" in
ESP32_S3 | ESP32_S3_EMULATED)
  run_pio_cmd "$PIO_EXE" device monitor &
  RUN_PID=$!
  ;;
*)
  echo "Error: invalid mode '$PROJECT_MODE'. Expected ESP32_S3 or ESP32_S3_EMULATED." >&2
  exit 1
  ;;
esac

# ----- Run In Background ----- #
if [[ "${RUN_IN_BACKGROUND:-}" == "true" ]]; then
  log DEBUG "Running in background with PID=$RUN_PID" &
  export RUN_PID=$RUN_PID
else
  log TRACE "Waiting for PID=$RUN_PID" &
  wait_on_pid_if_running $RUN_PID
fi
