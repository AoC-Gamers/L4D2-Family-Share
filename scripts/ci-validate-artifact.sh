#!/usr/bin/env bash

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
ARTIFACT_DIR="${SOURCEMOD_ARTIFACT_DIR:-$ROOT_DIR/dist/sourcemod/artifact}"
PACKAGE_MAP_PATH="$ROOT_DIR/plugin-package-map.json"

if [[ ! -d "$ARTIFACT_DIR" ]]; then
  echo "SourceMod artifact directory not found at $ARTIFACT_DIR" >&2
  exit 1
fi

PYTHON_BIN="$(command -v python3 >/dev/null 2>&1 && echo python3 || echo python)"

"$PYTHON_BIN" - "$ROOT_DIR" "$ARTIFACT_DIR" "$PACKAGE_MAP_PATH" <<'PY'
import json
import os
import sys


def validate_manifest_tree(source_root: str, artifact_root: str, manifest: dict) -> None:
    if manifest.get("all", False):
        if not os.path.isdir(source_root):
            raise SystemExit(f"Missing source directory for validation: {source_root}")
        if not os.path.isdir(artifact_root):
            raise SystemExit(f"Missing artifact directory: {artifact_root}")
        source_entries = sorted(os.listdir(source_root))
        artifact_entries = sorted(os.listdir(artifact_root))
        if source_entries != artifact_entries:
            raise SystemExit(f"Directory mismatch for {artifact_root}: expected {source_entries}, got {artifact_entries}")

    for relative_file in manifest.get("files", []):
        source_path = os.path.join(source_root, relative_file)
        artifact_path = os.path.join(artifact_root, relative_file)
        if not os.path.isfile(source_path):
            raise SystemExit(f"Missing source artifact file declared in manifest: {source_path}")
        if not os.path.isfile(artifact_path):
            raise SystemExit(f"Missing packaged artifact file: {artifact_path}")

    for relative_dir in manifest.get("dirs", []):
        source_path = os.path.join(source_root, relative_dir)
        artifact_path = os.path.join(artifact_root, relative_dir)
        if not os.path.isdir(source_path):
            raise SystemExit(f"Missing source artifact directory declared in manifest: {source_path}")
        if not os.path.isdir(artifact_path):
            raise SystemExit(f"Missing packaged artifact directory: {artifact_path}")

    for key, value in manifest.items():
        if key in {"all", "files", "dirs"}:
            continue
        if isinstance(value, dict):
            validate_manifest_tree(os.path.join(source_root, key), os.path.join(artifact_root, key), value)


root_dir, artifact_dir, package_map_path = sys.argv[1], sys.argv[2], sys.argv[3]

with open(package_map_path, "r", encoding="utf-8") as fh:
    package_map = json.load(fh)

artifact_plugins_dir = os.path.join(artifact_dir, "addons", "sourcemod", "plugins")
expected_plugins = {}
for bucket, plugins in package_map.get("build", {}).get("plugins", {}).items():
    for plugin in plugins:
        expected_plugins[plugin] = bucket

for plugin, bucket in expected_plugins.items():
    compiled_path = (
        os.path.join(artifact_plugins_dir, f"{plugin}.smx")
        if bucket == "root"
        else os.path.join(artifact_plugins_dir, bucket, f"{plugin}.smx")
    )
    if not os.path.isfile(compiled_path):
        raise SystemExit(f"Missing compiled plugin: {compiled_path}")

source_root = os.path.join(root_dir, "addons", "sourcemod")
artifact_root = os.path.join(artifact_dir, "addons", "sourcemod")
artifact_manifest = package_map.get("artifact", {}).get("addons", {}).get("sourcemod", {})
validate_manifest_tree(source_root, artifact_root, artifact_manifest)

include_dir = os.path.join(artifact_root, "scripting", "include")
allowed_include_files = {
    os.path.join(include_dir, "l4d2_familyshare.inc"),
}
if os.path.isdir(include_dir):
    for root, _, files in os.walk(include_dir):
        for name in files:
            path = os.path.join(root, name)
            if path not in allowed_include_files:
                raise SystemExit(f"Artifact includes unsupported complementary library: {path}")

for rel_path in ("README.md", "CHANGELOG.md", "plugin-package-map.json", "compile.log"):
    artifact_path = os.path.join(artifact_dir, rel_path)
    if not os.path.exists(artifact_path):
        raise SystemExit(f"Missing packaged project asset: {artifact_path}")

print("ARTIFACT_VALIDATION_OK")
PY
