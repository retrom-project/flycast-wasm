import json
import shutil
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path


class CandidateDescriptorTest(unittest.TestCase):
    def test_detached_ci_checkout_keeps_exact_clean_commit_identity(self):
        source = Path(__file__).with_name('candidate_descriptor.py')
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary) / 'source'
            scripts = root / '.github/rpg-runtime'
            scripts.mkdir(parents=True)
            shutil.copyfile(source, scripts / source.name)
            config = {'releaseAssets': ['core.data', 'rpg-runtime-release.json'],
                      'forkRepository': 'https://github.com/retrom-project/flycast-wasm',
                      'adapterAbi': 'emulatorjs-flycast-state-v1'}
            (root / 'retrom-fork.json').write_text(json.dumps(config))
            def git(*args):
                return subprocess.check_output(['git', '-C', str(root), *args], text=True).strip()
            git('init', '-q')
            git('add', '.')
            git('-c', 'user.name=Test', '-c', 'user.email=test@example.invalid', 'commit', '-qm', 'fixture')
            git('checkout', '--detach', '-q')
            output = Path(temporary) / 'candidate'
            output.mkdir()
            (output / 'core.data').write_bytes(b'owned synthetic core')
            subprocess.run([sys.executable, str(scripts / source.name), 'finalize', str(output),
                            '--core-id', 'flycast'], check=True, capture_output=True)
            descriptor = json.loads((output / 'retrom-core-candidate.json').read_text())
            self.assertEqual(descriptor['commit'], git('rev-parse', 'HEAD'))
            self.assertEqual(descriptor['branch'], 'HEAD')
            self.assertIs(descriptor['dirty'], False)
