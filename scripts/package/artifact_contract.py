"""Resolve the installable files from the project's package map."""

import argparse
import bz2
import filecmp
import json
import shutil
from pathlib import Path

MAP_PATH = Path("scripts/package/package-map.json")
SCRIPTING = Path("addons/sourcemod/scripting")
PLUGINS = Path("addons/sourcemod/plugins")
REQUIRED_PROJECT_FILES = (Path("README.md"), Path("LICENSE"), Path("NOTICES.md"))
CHANGELOG = Path("CHANGELOG.md")


def load_map(root: Path) -> dict:
    with (root / MAP_PATH).open(encoding="utf-8") as manifest:
        return json.load(manifest)


def relative_path(value: str) -> Path:
    path = Path(value)
    if not path.parts or path.is_absolute() or ".." in path.parts or any(char in value for char in ":\\"):
        raise ValueError(f"Invalid path in {MAP_PATH}: {value}")
    return path


def plugin_paths(manifest: dict) -> dict[Path, Path]:
    plugins = {}
    for bucket, names in manifest["build"]["plugins"].items():
        destination = PLUGINS if bucket == "root" else PLUGINS / relative_path(bucket)
        for name in names:
            binary = destination / relative_path(f"{name}.smx")
            if binary in plugins:
                raise ValueError(f"Duplicate plugin path in {MAP_PATH}: {binary}")
            plugins[binary] = SCRIPTING / relative_path(f"{name}.sp")
    if not plugins:
        raise ValueError(f"No product plugins in {MAP_PATH}")
    return plugins


def directory_files(root: Path, relative: Path):
    directory = root / relative
    if not directory.is_dir():
        raise FileNotFoundError(f"Missing declared directory: {directory}")
    yield from (path.relative_to(root) for path in sorted(directory.rglob("*")) if path.is_file())


def manifest_files(root: Path, tree: dict, prefix: Path = Path()):
    if tree.get("all"):
        yield from directory_files(root, prefix)
    for name in tree.get("files", []):
        path = prefix / relative_path(name)
        if not (root / path).is_file():
            raise FileNotFoundError(f"Missing declared file: {root / path}")
        yield path
    for name in tree.get("dirs", []):
        yield from directory_files(root, prefix / relative_path(name))
    for key, subtree in tree.items():
        if key not in {"all", "files", "dirs"}:
            yield from manifest_files(root, subtree, prefix / relative_path(key))


def product_files(root: Path, manifest: dict) -> tuple[set[Path], dict[Path, Path]]:
    sources = set(manifest_files(root, manifest["artifact"]))
    sidecar_roots = {relative_path(name) for name in manifest["build"].get("bz2_sidecars", [])}
    sidecars = {
        path.with_name(path.name + ".bz2"): path
        for path in sources
        if any(path.is_relative_to(directory) for directory in sidecar_roots)
    }
    for source in plugin_paths(manifest).values():
        if source not in sources:
            raise ValueError(f"Missing SourcePawn entrypoint in artifact declaration: {source}")
    if sources.intersection(sidecars):
        raise ValueError("Generated bz2 files collide with declared product files")
    return sources, sidecars


def copy_product_files(root: Path, destination: Path, manifest: dict) -> None:
    sources, sidecars = product_files(root, manifest)
    for path in sorted(sources):
        target = destination / path
        target.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(root / path, target)
    for target, source in sorted(sidecars.items()):
        with (root / source).open("rb") as original, bz2.open(destination / target, "wb") as compressed:
            shutil.copyfileobj(original, compressed)


def project_files(root: Path) -> tuple[Path, ...]:
    for path in REQUIRED_PROJECT_FILES:
        file = root / path
        if not file.is_file():
            raise FileNotFoundError(f"Missing required project file: {path}")
        if not file.read_bytes().strip():
            raise ValueError(f"Required project file is empty: {path}")

    license_text = (root / "LICENSE").read_bytes()
    if not all(marker in license_text for marker in (
        b"GNU GENERAL PUBLIC LICENSE",
        b"Version 3, 29 June 2007",
        b"END OF TERMS AND CONDITIONS",
    )):
        raise ValueError("LICENSE must contain the full GNU GPL v3 text")

    if (root / CHANGELOG).is_file():
        return (*REQUIRED_PROJECT_FILES, CHANGELOG)
    return REQUIRED_PROJECT_FILES


def validate(root: Path, artifact: Path) -> None:
    if not artifact.is_dir():
        raise FileNotFoundError(f"Missing artifact directory: {artifact}")
    manifest = load_map(root)
    sources, sidecars = product_files(root, manifest)
    legal_and_readme = set(project_files(root))
    plugins = set(plugin_paths(manifest))
    expected = sources | set(sidecars) | plugins | legal_and_readme
    actual = {path.relative_to(artifact) for path in artifact.rglob("*") if path.is_file()}
    if unexpected := actual - expected:
        raise ValueError(f"Artifact includes unsupported complementary library or file: {sorted(map(str, unexpected))}")
    if missing := expected - actual:
        raise ValueError(f"Missing packaged files: {sorted(map(str, missing))}")
    if len({path.as_posix().casefold() for path in expected}) != len(expected):
        raise ValueError("Paths in the artifact differ only by case")
    for path in sorted(sources | legal_and_readme):
        if not filecmp.cmp(root / path, artifact / path, shallow=False):
            raise ValueError(f"Packaged file differs from its source: {path}")
    for path, source in sorted(sidecars.items()):
        with bz2.open(artifact / path, "rb") as compressed:
            if compressed.read() != (root / source).read_bytes():
                raise ValueError(f"Compressed asset differs from its source: {path}")
    for path in sorted(plugins):
        with (artifact / path).open("rb") as binary:
            if binary.read(4) != b"FFPS":
                raise ValueError(f"Invalid SourcePawn plugin header: {path}")


def main() -> None:
    parser = argparse.ArgumentParser(description="Validate a staged SourceMod artifact.")
    parser.add_argument("--root", default=".", type=Path)
    parser.add_argument("--artifact", default=Path("dist/sourcemod/artifact"), type=Path)
    args = parser.parse_args()
    try:
        validate(args.root, args.artifact)
    except (OSError, ValueError) as error:
        parser.exit(1, f"{error}\n")
    print("ARTIFACT_VALIDATION_OK")


if __name__ == "__main__":
    main()
