"""Validate declared nested includes without permitting extra libraries."""
import os
import shutil
import sys
from pathlib import Path
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "scripts/package"))
from artifact_contract import copy_product_files, load_map, plugin_paths, project_files


class ArtifactManifest(unittest.TestCase):
    def setUp(self):
        (ROOT/'tmp').mkdir(exist_ok=True)
        self.temp = tempfile.TemporaryDirectory(dir=ROOT/'tmp')
        self.addCleanup(self.temp.cleanup)
        self.artifact = Path(self.temp.name)/'artifact'
        manifest = load_map(ROOT)
        copy_product_files(ROOT, self.artifact, manifest)
        for binary in plugin_paths(manifest):
            path = self.artifact / binary
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_bytes(b"FFPSpackaging fixture")
        for path in project_files(ROOT):
            shutil.copy2(ROOT / path, self.artifact / path)
        sm = self.artifact / "addons/sourcemod"
        self.includes = sm/'scripting/include'

    def validate(self):
        return subprocess.run(['make', '-f', str(ROOT / 'Makefile'), 'validate-artifact'],
            env=dict(os.environ, SOURCEMOD_ARTIFACT_DIR=str(self.artifact)), capture_output=True, text=True)

    def test_declared_nested_library_is_accepted(self):
        result = self.validate()
        self.assertEqual(result.returncode, 0, result.stdout+result.stderr)

    def test_missing_legal_document_is_rejected(self):
        for name in ("LICENSE", "NOTICES.md"):
            with self.subTest(document=name):
                document = self.artifact / name
                document.unlink()
                try:
                    result = self.validate()
                    self.assertNotEqual(result.returncode, 0)
                    self.assertIn(f"Missing packaged files: ['{name}']", result.stderr)
                finally:
                    shutil.copy2(ROOT / name, document)

    def test_placeholder_license_is_rejected(self):
        self.artifact.joinpath("LICENSE").write_text("License pending")
        result = self.validate()
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("Packaged file differs from its source: LICENSE", result.stderr)

    def test_undeclared_root_library_is_rejected(self):
        (self.includes/'unlisted.inc').write_text('unexpected')
        result = self.validate()
        self.assertNotEqual(result.returncode, 0)
        self.assertIn('unsupported complementary library', result.stderr)

    def test_undeclared_nested_library_is_rejected(self):
        (self.includes/'steamidtools/unlisted.inc').write_text('unexpected')
        result = self.validate()
        self.assertNotEqual(result.returncode, 0)
        self.assertIn('unsupported complementary library', result.stderr)

    def test_missing_declared_nested_file_is_rejected(self):
        (self.includes/'steamidtools/clients.inc').unlink()
        result = self.validate()
        self.assertNotEqual(result.returncode, 0)
