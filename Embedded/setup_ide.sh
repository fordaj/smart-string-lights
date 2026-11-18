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

# ----- Run ----- #
log INFO "Setting up IDE in mode: $PROJECT_MODE" &
case "$PROJECT_MODE" in
ESP32_S3 | ESP32_S3_EMULATED)
  "$PIO_EXE" project init --ide vscode --environment "$PROJECT_MODE" # TODO: For some reason this only works with the pio system command on windows
  ;;
*)
  echo "Error: invalid mode '$PROJECT_MODE'. Expected ESP32_S3 or ESP32_S3_EMULATED." >&2
  exit 1
  ;;
esac
