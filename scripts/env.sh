#!/usr/bin/env bash
# scripts/env.sh — source the environment cascade.
#
#   source scripts/env.sh
#
# Sources scripts/.common.env (the base cascade) followed by the
# platform file scripts/.<os>-<arch>.env selected from `uname`
# (for example .macos-x86_64.env). The cascade files define the
# variables; this script only locates them, sources them, and
# anchors the repository-relative tool paths (tools/...) to the
# repository root so they resolve from any working directory.
# Works in bash and zsh, sourced from any directory.
#
# Only windows-x86_64 and macos-x86_64 are supported; every other
# platform is rejected with an error (matching the repository's
# supported target list).

# Resolve this file's directory (sourced or executed, in bash and zsh).
# In zsh $0 is the sourced file's path (FUNCTION_ARGZERO, the default).
# Kept bash-parseable on purpose: shfmt checks this repo's shell files
# as bash.
if [ -n "${BASH_VERSION:-}" ]; then
  _env_sh_path="${BASH_SOURCE[0]}"
else
  _env_sh_path="$0"
fi
ENV_DIR="$(cd -- "$(dirname -- "$_env_sh_path")" && pwd)"
REPO_ROOT_DIR="$(cd -- "$ENV_DIR/.." && pwd)"
unset _env_sh_path

set -a
# shellcheck disable=SC1091
. "$ENV_DIR/.common.env"
case "$(uname -s):$(uname -m)" in
Darwin:x86_64) . "$ENV_DIR/.macos-x86_64.env" ;;
MINGW*:x86_64 | MSYS*:x86_64 | CYGWIN*:x86_64) . "$ENV_DIR/.windows-x86_64.env" ;;
*)
  echo "Unsupported platform: $(uname -s) $(uname -m)" >&2
  set +a
  # shellcheck disable=SC2317
  return 1 2>/dev/null || exit 1
  ;;
esac

# Anchor the repository-relative tool binaries to the repo root.
# Absolute paths are left untouched.
case "${VERSIONER_BIN:-}" in "" | /*) ;; *) VERSIONER_BIN="$REPO_ROOT_DIR/$VERSIONER_BIN" ;; esac
case "${LOGAMUFFIN_BIN:-}" in "" | /*) ;; *) LOGAMUFFIN_BIN="$REPO_ROOT_DIR/$LOGAMUFFIN_BIN" ;; esac
case "${RUNNER_SPAWNER_BIN:-}" in "" | /*) ;; *) RUNNER_SPAWNER_BIN="$REPO_ROOT_DIR/$RUNNER_SPAWNER_BIN" ;; esac
case "${BARNACLE_BIN:-}" in "" | /*) ;; *) BARNACLE_BIN="$REPO_ROOT_DIR/$BARNACLE_BIN" ;; esac
case "${FORMATTINGTON_LITE_BIN:-}" in "" | /*) ;; *) FORMATTINGTON_LITE_BIN="$REPO_ROOT_DIR/$FORMATTINGTON_LITE_BIN" ;; esac
case "${PIO_EXE:-}" in "" | /*) ;; *) PIO_EXE="$REPO_ROOT_DIR/$PIO_EXE" ;; esac
case "${ESPTOOL_EXE:-}" in "" | /*) ;; *) ESPTOOL_EXE="$REPO_ROOT_DIR/$ESPTOOL_EXE" ;; esac
case "${PYTHON_EXE:-}" in "" | /*) ;; *) PYTHON_EXE="$REPO_ROOT_DIR/$PYTHON_EXE" ;; esac
case "${VENV:-}" in "" | /*) ;; *) VENV="$REPO_ROOT_DIR/${VENV#./}" ;; esac

set +a
