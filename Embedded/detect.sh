#!/usr/bin/env bash
set -euo pipefail

# --- Source Scripts --- #
(return 0 2>/dev/null) && sourced=true || sourced=false
if ! "${sourced}"; then
  source "$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)/helpers.sh"
fi

# --- Run esptool to check connected devices --- #
log DEBUG "Checking connected devices with esptool" &
LOG_PIDS+=($!)
set +e
output=$(NO_COLOR=1 $ESPTOOL_EXE read-mac 2>&1)
status=$?
set -e
# esptool exits in error if it cannot connect, so proceed even if an error occured
# if [[ $status -ne 0 ]]; then
#     log ERROR "esptool exited in error:\n$output"
#     wait "${LOG_PIDS[@]}"
#     exit $status
# fi
log TRACE "esptool output:\n$output" &
LOG_PIDS+=($!)

# --- Parse esptool output and populate variables --- #
log DEBUG "Parsing esptool output" &
LOG_PIDS+=($!)
IS_ESP32S3_CONNECTED=false
IS_ESP8266_CONNECTED=false
case "$output" in
*"ESP32-S3"*) IS_ESP32S3_CONNECTED=true ;;
*"ESP8266"*) IS_ESP8266_CONNECTED=true ;;
esac
DEVICE_STATUS_TABLE="\
| Device | Connected? |
| :---: | :---: |
| ESP32S3 | $IS_ESP32S3_CONNECTED |
| ESP8266 | $IS_ESP8266_CONNECTED |"
log TRACE "$DEVICE_STATUS_TABLE" &
LOG_PIDS+=($!)

# --- Conditionally Output to GitHub Actions --- #
if [[ "${GITHUB_ACTIONS:-}" == "true" ]]; then
  log TRACE "GitHub Actions environment detected, outputting to GITHUB_ENV and GITHUB_OUTPUT" &
  LOG_PIDS+=($!)
  export IS_ESP32S3_CONNECTED="$IS_ESP32S3_CONNECTED"
  export IS_ESP8266_CONNECTED="$IS_ESP8266_CONNECTED"
  echo "${GITHUB_OUTPUT:-}"
  echo "IS_ESP32S3_CONNECTED=$IS_ESP32S3_CONNECTED" >>"$GITHUB_ENV"
  echo "IS_ESP32S3_CONNECTED=$IS_ESP32S3_CONNECTED" >>"$GITHUB_OUTPUT"
  echo "IS_ESP8266_CONNECTED=$IS_ESP8266_CONNECTED" >>"$GITHUB_ENV"
  echo "IS_ESP8266_CONNECTED=$IS_ESP8266_CONNECTED" >>"$GITHUB_OUTPUT"
  echo "===== GITHUB_OUTPUT ====="
  cat "$GITHUB_OUTPUT"
  echo "========================="
fi

# --- Exit Successfully --- #
wait "${LOG_PIDS[@]}"
