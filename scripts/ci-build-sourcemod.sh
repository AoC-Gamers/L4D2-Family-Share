#!/usr/bin/env bash

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

cd "$ROOT_DIR"

echo "Resolving SourceMod dependencies through make..."
make deps-smx PYTHON=python3 SOURCEMOD_VERSION="${SOURCEMOD_VERSION:-1.12}" SMX_PLATFORM=linux

echo "Building SMX output through make..."
make build-smx PYTHON=python3 SPCOMP="deps/sourcemod-linux/addons/sourcemod/scripting/spcomp"

echo "Packaging SMX tree through make..."
make package-smx PYTHON=python3

echo "Staging final SourceMod artifact..."
python3 ./scripts/stage-artifact.py . ./.build/package-smx ./deps/build-smx-compile.log
