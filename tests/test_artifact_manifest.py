"""Validate declared nested includes without permitting extra libraries."""
import importlib.util
import json
import os
from pathlib import Path
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location('builder', ROOT/'scripts/build-local.py')
builder = importlib.util.module_from_spec(spec)
spec.loader.exec_module(builder)

class ArtifactManifest(unittest.TestCase):
    def setUp(self):
        (ROOT/'tmp').mkdir(exist_ok=True)
        self.temp = tempfile.TemporaryDirectory(dir=ROOT/'tmp')
        self.addCleanup(self.temp.cleanup)
        self.artifact = Path(self.temp.name)/'artifact'
        manifest = json.loads((ROOT/'plugin-package-map.json').read_text())
        sm = self.artifact/'addons/sourcemod'
        builder.copy_manifest_tree(manifest['artifact']['addons']['sourcemod'], ROOT/'addons/sourcemod', sm)
        for bucket, names in manifest['build']['plugins'].items():
            for name in names:
                p = sm/'plugins'/(name+'.smx') if bucket == 'root' else sm/'plugins'/bucket/(name+'.smx')
                p.parent.mkdir(parents=True, exist_ok=True)
                p.write_bytes(b'packaging fixture')
        for name in ['README.md', 'CHANGELOG.md', 'plugin-package-map.json', 'compile.log']:
            (self.artifact/name).write_text('fixture')
        self.includes = sm/'scripting/include'

    def validate(self):
        return subprocess.run(['bash', str(ROOT/'scripts/ci-validate-artifact.sh')],
            env=dict(os.environ, SOURCEMOD_ARTIFACT_DIR=str(self.artifact)), capture_output=True, text=True)

    def test_declared_nested_library_is_accepted(self):
        result = self.validate()
        self.assertEqual(result.returncode, 0, result.stdout+result.stderr)

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
