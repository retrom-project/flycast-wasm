"""Generate core-owned ROM requirements from the prepared compilation unit."""
import hashlib
import json
from pathlib import Path
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / ".cache/flycast"


def export(output: Path, core: Path):
    binary = ROOT / ".cache/export-rom-requirements"
    exporter = ROOT / ".github/rpg-runtime/export-rom-requirements.cpp"
    subprocess.run(["g++", "-std=c++17", "-O2", "-I" + str(SOURCE / "core"),
                    "-I" + str(SOURCE / "core/deps"), "-I" + str(SOURCE / "core/deps/nowide/include"),
                    str(exporter), "-o", str(binary)], check=True)
    machines = json.loads(subprocess.check_output([str(binary)]))
    # FindGame in naomi_cart.cpp returns the first matching table entry.
    # Preserve that exact selection even if upstream repeats a machine name.
    records = {}
    for entry in machines:
        records.setdefault(entry["name"], entry)
    machines = sorted(records.values(), key=lambda entry: entry["name"])
    if not machines:
        raise ValueError("FLYCAST_ROM_REQUIREMENTS_INVALID")
    if records["starseek"]["mediaType"] != "GDROM" or records["starseek"]["disc"] != "gdl-0005":
        raise ValueError("FLYCAST_GDROM_REQUIREMENTS_INVALID")
    for entry in machines:
        if any(file["sizeBytes"] <= 0 for file in entry["files"]):
            raise ValueError("FLYCAST_ROM_REQUIREMENTS_INVALID")
    tables = [SOURCE / "core/hw/naomi/naomi_roms.cpp", SOURCE / "core/hw/naomi/naomi_roms.h"]
    value = {"schemaVersion": 1, "kind": "FLYCAST_ROM_REQUIREMENTS", "core": {
        "filename": core.name, "sha256": hashlib.sha256(core.read_bytes()).hexdigest(),
        "sourceCommit": subprocess.check_output(["git", "rev-parse", "HEAD"], cwd=SOURCE, text=True).strip(),
        "tableSha256": hashlib.sha256(b"".join(path.read_bytes() for path in tables)).hexdigest(),
        "exporterSha256": hashlib.sha256(exporter.read_bytes()).hexdigest(),
    }, "machines": machines}
    output.write_text(json.dumps(value, sort_keys=True, separators=(",", ":")) + "\n")


if __name__ == "__main__":
    export(Path(sys.argv[1]), Path(sys.argv[2]))
