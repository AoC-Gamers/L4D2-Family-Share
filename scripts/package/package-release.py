#!/usr/bin/env python3

import argparse
import os
import re
import shutil
import tempfile
from pathlib import Path
from zipfile import ZipFile

from artifact_contract import validate


def main() -> int:
    parser = argparse.ArgumentParser(description="Create a validated SourceMod installable ZIP.")
    parser.add_argument("--root", default=".", type=Path)
    parser.add_argument("--artifact", type=Path)
    parser.add_argument("--basename", required=True)
    args = parser.parse_args()
    if not re.fullmatch(r"[A-Za-z0-9][A-Za-z0-9._-]*", args.basename):
        parser.error("expected a ZIP base name without path separators")

    artifact = args.artifact or args.root / "dist/sourcemod/artifact"
    validate(args.root, artifact)
    release_dir = args.root / "dist/release"
    release_dir.mkdir(parents=True, exist_ok=True)
    destination = release_dir / f"{args.basename}.zip"

    with tempfile.TemporaryDirectory(dir=release_dir) as temporary:
        archive = shutil.make_archive(str(Path(temporary) / args.basename), "zip", root_dir=artifact)
        with ZipFile(archive) as package:
            expected = {path.relative_to(artifact).as_posix() for path in artifact.rglob("*") if path.is_file()}
            files = [entry.filename for entry in package.infolist() if not entry.is_dir()]
            if len(files) != len(expected) or set(files) != expected or package.testzip() is not None:
                raise ValueError("The generated ZIP does not match the validated staging tree")
        os.replace(archive, destination)

    print(f"Release archive generated in: {destination}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
