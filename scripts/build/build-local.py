#!/usr/bin/env python3

import argparse
import shutil
import subprocess
import sys
import time
from pathlib import Path


sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "package"))
from artifact_contract import copy_product_files, load_map, plugin_paths


def run_spcomp(spcomp: Path, source_file: Path, include_dirs: list[Path], output_file: Path, compile_log: Path) -> None:
    cmd = [str(spcomp), str(source_file)]
    for include_dir in include_dirs:
        cmd.append(f"-i{include_dir}")
    cmd.append(f"-o{output_file}")

    rel_output = output_file.as_posix().split("/addons/", 1)[-1]
    print(f"Compiling {source_file.name} -> addons/{rel_output}", flush=True)
    result = subprocess.run(cmd, capture_output=True, text=True, check=False)

    if result.stdout:
        print(result.stdout, end="")
    if result.stderr:
        print(result.stderr, end="", file=sys.stderr)

    with compile_log.open("a", encoding="utf-8", newline="") as fh:
        if result.stdout:
            fh.write(result.stdout)
        if result.stderr:
            fh.write(result.stderr)

    if result.returncode != 0:
        raise RuntimeError(f"spcomp failed for {source_file.name}")

    if not output_file.exists():
        raise RuntimeError(f"Expected output file was not generated: {output_file}")


def remove_tree_if_exists(target: Path) -> None:
    if not target.exists():
        return

    last_error = None
    for _ in range(3):
        try:
            shutil.rmtree(target)
            return
        except FileNotFoundError:
            return
        except OSError as exc:
            last_error = exc
            time.sleep(0.2)

    if target.exists() and last_error is not None:
        raise last_error


def detect_default_workspace(root: Path) -> Path | None:
    if root.as_posix().startswith("/mnt/"):
        return Path("/tmp/l4d2-family-share-build")
    return None


def main() -> int:
    parser = argparse.ArgumentParser(description="Compile L4D2-Family-Share SourceMod plugins into a local build directory.")
    parser.add_argument("--root", default=".", help="Repository root")
    parser.add_argument("--spcomp", required=True, help="Path to spcomp executable")
    parser.add_argument("--output-root", default="build", help="Output directory relative to repo root")
    parser.add_argument("--compile-log", required=True, help="Compile log path relative to repo root")
    parser.add_argument("--workspace", default="", help="Optional temporary workspace directory")
    args = parser.parse_args()

    root = Path(args.root).resolve()
    spcomp = Path(args.spcomp).resolve()
    output_root = (root / args.output_root).resolve()
    compile_log = (root / args.compile_log).resolve()
    workspace = Path(args.workspace).resolve() if args.workspace else detect_default_workspace(root)

    if not spcomp.exists():
        raise FileNotFoundError(f"spcomp not found: {spcomp}")
    spcomp.chmod(spcomp.stat().st_mode | 0o111)

    source_mod_include_dir = spcomp.parent / "include"
    if not source_mod_include_dir.exists():
        raise FileNotFoundError(f"SourceMod include dir not found: {source_mod_include_dir}")

    manifest = load_map(root)
    plugins = plugin_paths(manifest)
    source_root = root / "addons" / "sourcemod"
    if output_root == root or output_root in root.parents:
        raise ValueError(f"Build output would overwrite the repository: {output_root}")
    if output_root == source_root or output_root in source_root.parents or source_root in output_root.parents:
        raise ValueError(f"Build output overlaps the SourceMod tree: {output_root}")
    scripting_dir = source_root / "scripting"
    include_dir = scripting_dir / "include"

    if workspace is not None:
        print(f"Using temporary workspace: {workspace}", flush=True)
        remove_tree_if_exists(workspace)
        workspace.mkdir(parents=True, exist_ok=True)
        workspace_source_root = workspace / "addons" / "sourcemod"
        shutil.copytree(source_root, workspace_source_root, dirs_exist_ok=True)
        workspace_spcomp_dir = workspace / "spcomp"
        workspace_spcomp_dir.mkdir(parents=True, exist_ok=True)
        workspace_spcomp = workspace_spcomp_dir / spcomp.name
        shutil.copy2(spcomp, workspace_spcomp)
        workspace_spcomp.chmod(workspace_spcomp.stat().st_mode | 0o111)
        shutil.copytree(source_mod_include_dir, workspace_spcomp_dir / "include", dirs_exist_ok=True)
        source_root = workspace_source_root
        scripting_dir = source_root / "scripting"
        include_dir = scripting_dir / "include"
        spcomp = workspace_spcomp
        source_mod_include_dir = workspace_spcomp_dir / "include"

    remove_tree_if_exists(output_root)
    compile_log.parent.mkdir(parents=True, exist_ok=True)
    compile_log.write_text("", encoding="utf-8")

    include_dirs = [include_dir, scripting_dir, source_mod_include_dir]
    for binary, source in plugins.items():
        output_file = output_root / binary
        output_file.parent.mkdir(parents=True, exist_ok=True)
        run_spcomp(spcomp, scripting_dir / source.name, include_dirs, output_file, compile_log)

    copy_product_files(root, output_root, manifest)

    if workspace is not None and workspace.exists():
        remove_tree_if_exists(workspace)

    print()
    print(f"Build local completed in: {output_root}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
