"""Package the locally compiled EmulatorJS core with a closed member set."""
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile

root = Path(__file__).resolve().parents[2]
output = Path(sys.argv[1]).resolve()
assert output.is_dir() and not any(output.iterdir())
with tempfile.TemporaryDirectory(dir=root / ".cache", prefix="package-") as temporary:
    stage = Path(temporary)
    for name in ("flycast_libretro.js", "flycast_libretro.wasm"):
        shutil.copyfile(root / ".cache/flycast/build-wasm-prod" / name, stage / name)
    (stage / "build.json").write_text(json.dumps({"minimumEJSVersion": "4.2.3", "version": "1.0"}) + "\n")
    core = json.loads((root / "config/core.json").read_text())
    core["repo"] = "https://github.com/retrom-project/flycast-wasm"
    (stage / "core.json").write_text(json.dumps(core) + "\n")
    notices = (root / "LICENSE").read_bytes() + b"\nRetroArch frontend:\n" + (root / ".cache/retroarch/COPYING").read_bytes()
    for name in ("libzip/LICENSE", "miniupnpc/LICENSE", "libchdr/LICENSE.txt", "libchdr/deps/lzma-24.05/LICENSE", "libchdr/deps/zstd-1.5.6/LICENSE", "libchdr/deps/zlib-1.3.1/LICENSE", "xxHash/LICENSE", "nowide/LICENSE"):
        notices += b"\n" + name.encode() + b":\n" + (root / ".cache/flycast/core/deps" / name).read_bytes()
    (stage / "license.txt").write_bytes(notices)
    for path in stage.iterdir():
        os.chmod(path, 0o644)
        os.utime(path, (0, 0))
    subprocess.run(["7z", "a", "-mtm=off", "-mta=off", "-mtc=off", "-bd", "-bso0", "-bsp0", "-t7z",
                    str(output / "flycast-wasm.data"), *sorted(p.name for p in stage.iterdir())], cwd=stage, check=True)
(output / "LICENSE").write_bytes(notices)
(output / "flycast.json").write_text(json.dumps({"core": "flycast", "buildStart": "retrom-flycast-1.0", "options": {"defaultWebGL2": True}}) + "\n")
subprocess.run([sys.executable, str(root / ".github/rpg-runtime/candidate_descriptor.py"),
                "finalize", str(output), "--core-id", "flycast"], check=True)
