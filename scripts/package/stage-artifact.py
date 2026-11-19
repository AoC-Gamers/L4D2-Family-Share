#!/usr/bin/env python3

import shutil
import sys
from pathlib import Path

from artifact_contract import project_files, validate


def main() -> int:
    if len(sys.argv) not in (4, 5):
        raise SystemExit("Usage: stage-artifact.py <root_dir> <build_dir> <compile_log> [output_dir]")

    root = Path(sys.argv[1]).resolve()
    build = Path(sys.argv[2]).resolve()
    # Keep the compile_log argument for existing callers. CI archives the log separately.
    output = Path(sys.argv[4]).resolve() if len(sys.argv) == 5 else root / "dist/sourcemod/artifact"
    source_trees = tuple(root / name for name in ("addons", "cfg", "models", "sound"))
    if output == root or output in root.parents or any(
        output == path or path in output.parents for path in source_trees
    ):
        raise SystemExit(f"Staging would overwrite the repository: {output}")
    if output == build or output in build.parents or build in output.parents:
        raise SystemExit(f"Staging overlaps the compiled tree: {build} -> {output}")
    if not (build / "addons").is_dir():
        raise SystemExit(f"Missing compiled tree: {build}")

    if output.exists():
        shutil.rmtree(output)
    output.mkdir(parents=True)
    for name in ("addons", "cfg", "models", "sound"):
        if (build / name).is_dir():
            shutil.copytree(build / name, output / name)
    for path in project_files(root):
        shutil.copy2(root / path, output / path)

    validate(root, output)
    print(f"SourceMod artifacts generated in {output}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
