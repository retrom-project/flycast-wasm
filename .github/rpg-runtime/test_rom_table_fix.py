"""The core patch must correct the affected chip without changing its neighbor."""
from pathlib import Path
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[2]
TABLE = '''            { "mpr-22177.ic14s", 0x7000000, 0x800000, 0x15514cbc },
            { "mpr-22178.ic15s", 0x7800000, 0x800000, 0x9ea0552f },
            { "mpr-22179.ic16s", 0x8000000, 0x800000, 0x6915c4e6 },
            { "mpr-22180.ic17s", 0x8800000, 0x800000, 0x6915c4e6 },
            { "mpr-22181.ic18s", 0x9000000, 0x800000, 0x5a39b68e },
            { "mpr-22182.ic19s", 0x9800000, 0x800000, 0xc5606c42 },
            { "mpr-22183.ic20s", 0xa000000, 0x800000, 0x776af308 },
'''


class ROMTableFixTest(unittest.TestCase):
    def test_preparation_applies_the_crc_fix_to_the_compiled_table(self):
        patch = ROOT / "patches/flycast-rom-crc.patch"
        # Preparation is the common candidate and release input path.
        self.assertIn(patch.name, (ROOT / ".github/rpg-runtime/prepare-source.py").read_text())
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            target = root / "core/hw/naomi/naomi_roms.cpp"
            target.parent.mkdir(parents=True)
            target.write_text(TABLE)
            subprocess.run(["git", "apply", str(patch)], cwd=root, check=True)
            actual = target.read_text()
        self.assertIn('"mpr-22180.ic17s", 0x8800000, 0x800000, 0x744c3a40', actual)
        self.assertIn('"mpr-22179.ic16s", 0x8000000, 0x800000, 0x6915c4e6', actual)
        self.assertEqual(actual, TABLE.replace('"mpr-22180.ic17s", 0x8800000, 0x800000, 0x6915c4e6',
                                               '"mpr-22180.ic17s", 0x8800000, 0x800000, 0x744c3a40'))
