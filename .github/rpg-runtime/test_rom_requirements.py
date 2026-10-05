"""Exercise exported media/file semantics with project-owned table values."""
import json
from pathlib import Path
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[2]

TABLE = r'''
enum BlobType { Normal, Copy, Eeprom, EepromBE16 };
enum CartridgeType { M2, AW, GD };
struct Blob { const char* filename; unsigned length; unsigned crc; BlobType blob_type; };
struct Game { const char* name; const char* parent_name; const char* bios; CartridgeType cart_type; const char* gdrom_name; Blob blobs[5]; };
Game Games[] = {
 {"cart", "parent", "naomi2", M2, nullptr, {{"rom.bin", 4, 0x12, Normal}, {"virtual", 4, 0, Copy}, {"default.nv", 2, 0, Eeprom}, {"required.nv", 2, 0x34, EepromBE16}, {}}},
 {"disc", nullptr, nullptr, GD, "disc-id", {{"security.pic", 8, 0x56, Normal}, {}}},
 {"atom", nullptr, nullptr, AW, nullptr, {{"quote\".bin", 16, 0, Normal}, {}}},
 {"systemsp", nullptr, "segasp", M2, "compact-flash", {{"security.bin", 8, 0x78, Normal}, {}}},
 {}
};
'''


class ROMRequirementsTest(unittest.TestCase):
    def test_exports_required_optional_virtual_and_media_facts(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            table = root / "hw/naomi/naomi_roms.cpp"
            table.parent.mkdir(parents=True)
            table.write_text(TABLE)
            binary = root / "export"
            subprocess.run(["g++", "-std=c++17", "-I" + str(root), str(ROOT / ".github/rpg-runtime/export-rom-requirements.cpp"), "-o", str(binary)], check=True)
            machines = json.loads(subprocess.check_output([str(binary)]))
        cart, disc, atom, systemsp = machines
        self.assertEqual((cart["platform"], cart["parent"], cart["mediaType"]), ("naomi2", "parent", "CARTRIDGE"))
        self.assertEqual([file["name"] for file in cart["files"]], ["rom.bin", "default.nv", "required.nv"])
        self.assertEqual([file["optional"] for file in cart["files"]], [False, True, False])
        self.assertEqual(cart["files"][0]["crc32"], "00000012")
        self.assertEqual((disc["platform"], disc["mediaType"], disc["disc"]), ("naomi", "GDROM", "disc-id"))
        self.assertEqual(atom["platform"], "atomiswave")
        self.assertEqual(atom["files"], [{"name": 'quote".bin', "sizeBytes": 16, "crc32": None, "optional": False}])
        self.assertEqual((systemsp["platform"], systemsp["mediaType"], systemsp["disc"]),
                         ("systemsp", "COMPACT_FLASH", "compact-flash"))
