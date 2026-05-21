#!/usr/bin/env bash

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
RELEASE_BASENAME="${RELEASE_BASENAME:?RELEASE_BASENAME is required}"

cd "$ROOT_DIR"

make release PYTHON=python3 RELEASE_BASENAME="$RELEASE_BASENAME"
