#!/usr/bin/env bash
# scripts/format.sh — run formattington with logging via logamuffin.
#
# Usage:
#   ./scripts/format.sh [--check] [...]
#
# Forwards all arguments to formattington `<args> all
# --config .formattington.json`. The primary path is
# `uv run formattington` (when available in the project environment);
# otherwise the pinned `tools/formattington-lite` bundle is used as the
# fallback. Runs from the repository root so the config path resolves
# regardless of the caller's directory.

set -euo pipefail

# This script lives in scripts/, so the repository root is its parent directory.
SCRIPT_DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." &>/dev/null && pwd)
if [[ ! -f "$SCRIPT_DIR/scripts/entry.sh" ]]; then
  printf 'scripts/format.sh: error: env cascade not found: %s/scripts/entry.sh\n' "$SCRIPT_DIR" >&2
  exit 1
fi
# shellcheck disable=SC1091
. "$SCRIPT_DIR/scripts/entry.sh"

cd "$SCRIPT_DIR"

if command -v uv >/dev/null 2>&1 && uv run formattington --version >/dev/null 2>&1; then
  log INFO "running formattington via uv (uv run formattington $* all --config .formattington.json)"
  uv run formattington "$@" all --config .formattington.json
else
  if [[ -z "${FORMATTINGTON_LITE_BIN:-}" || ! -x "${FORMATTINGTON_LITE_BIN:-}" ]]; then
    log ERROR "no formattington available: 'uv run formattington' failed and tools/formattington-lite bundle is missing (run tools/formattington-lite/update.sh or set FORMATTINGTON_LITE_BIN)"
    exit 1
  fi
  log INFO "running formattington-lite ($FORMATTINGTON_LITE_BIN $* all --config .formattington.json)"
  "$FORMATTINGTON_LITE_BIN" "$@" all --config .formattington.json
fi
log INFO "format complete"
