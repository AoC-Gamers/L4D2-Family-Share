#!/usr/bin/env bash

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
ARTIFACT_DIR="${SOURCEMOD_ARTIFACT_DIR:-$ROOT_DIR/dist/sourcemod/artifact}"

if [[ ! -d "$ARTIFACT_DIR" ]]; then
  echo "SourceMod artifact directory not found at $ARTIFACT_DIR" >&2
  exit 1
fi

python3 - "$ARTIFACT_DIR" <<'PY'
import os
import sys

artifact_dir = sys.argv[1]
sm_dir = os.path.join(artifact_dir, "addons", "sourcemod")

expected_files = [
    os.path.join(sm_dir, "plugins", "l4d2_familyshare.smx"),
    os.path.join(sm_dir, "scripting", "l4d2_familyshare.sp"),
    os.path.join(sm_dir, "translations", "l4d2_familyshare.phrases.txt"),
    os.path.join(sm_dir, "translations", "es", "l4d2_familyshare.phrases.txt"),
    os.path.join(sm_dir, "configs", "sql-init", "mysql", "l4d2_familyshare.sql"),
    os.path.join(artifact_dir, "compile.log"),
]

for path in expected_files:
    if not os.path.isfile(path):
        raise SystemExit(f"Missing artifact file: {path}")

include_dir = os.path.join(sm_dir, "scripting", "include")
if os.path.exists(include_dir):
    raise SystemExit(f"Artifact must not include scripting/include: {include_dir}")

print("ARTIFACT_VALIDATION_OK")
PY
