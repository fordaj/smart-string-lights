#!/usr/bin/env bash
set -euo pipefail

# --- Source Scripts --- #
(return 0 2>/dev/null) && sourced=true || sourced=false
if ! "${sourced}"; then
  source "$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)/helpers.sh"
fi

# --- Run the host-side unit tests --- #
log INFO "Running unit tests (host)" &
LOG_PIDS+=($!)
run_pio_cmd "$PIO_EXE" test -e host
wait "${LOG_PIDS[@]}"
