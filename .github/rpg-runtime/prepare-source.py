"""Materialize exact upstream revisions in this fork's disposable build cache."""
from pathlib import Path
import subprocess
import shutil
import hashlib
import json

ROOT = Path(__file__).resolve().parents[2]
PINS = {
    "flycast": ("https://github.com/flyinghead/flycast", "2c48c0188a2afc158b02b6d1865d898756a03071"),
    "retroarch": ("https://github.com/EmulatorJS/RetroArch", "6dd4353937ef48b6ec0bfbdbb15d1c5992d86927"),
}


def git(path, *args):
    subprocess.run(["git", *args], cwd=path, check=True)


def prepare(name, repository, commit):
    path = ROOT / ".cache" / name
    if not path.exists():
        path.parent.mkdir(parents=True, exist_ok=True)
        git(ROOT, "clone", "--filter=blob:none", "--no-checkout", repository, str(path))
        git(path, "fetch", "origin", commit)
        git(path, "checkout", "--detach", commit)
    actual = subprocess.check_output(["git", "rev-parse", "HEAD"], cwd=path, text=True).strip()
    if actual != commit:
        raise RuntimeError(f"PINNED_SOURCE_MISMATCH:{name}")
    # The preparation below verifies/restores tracked generated inputs when needed.
    git(path, "submodule", "update", "--init", "--recursive")
    return path


for name, (repository, commit) in PINS.items():
    source = prepare(name, repository, commit)
    if name != "flycast":
        git(source, "diff", "--quiet", "HEAD", "--")
        continue
    stamp = ROOT / ".cache/prepared-source.json"
    def identity():
        diff = subprocess.check_output(["git", "diff", "--binary", "HEAD"], cwd=source)
        patches = b"".join(path.name.encode() + path.read_bytes() for path in sorted((ROOT / "patches").iterdir()) if path.is_file())
        generated = b"".join(path.name.encode() + path.read_bytes() for path in sorted((source / "core/rec-wasm").glob("*")) if path.is_file())
        return {"patches": hashlib.sha256(patches).hexdigest(), "tree": hashlib.sha256(diff + generated).hexdigest()}
    if stamp.exists() and json.loads(stamp.read_text()) == identity():
        continue
    git(source, "restore", "--source", commit, "--worktree", "--", ".")
    # Upstream tracks CRLF; the published patch uses LF.
    audio = source / "shell/libretro/audiostream.cpp"
    audio.write_bytes(audio.read_bytes().replace(b"\r\n", b"\n"))
    patch = ROOT / "patches/wasm-jit-phase1-modified.patch"
    git(source, "apply", "--check", str(patch))
    git(source, "apply", str(patch))
    (source / "core/rec-wasm").mkdir(exist_ok=True)
    for path in (ROOT / "patches").iterdir():
        if path.suffix in {".cpp", ".h"}:
            target = source / "core/rec-wasm" / path.name
            if not target.exists() or target.read_bytes() != path.read_bytes():
                shutil.copyfile(path, target)

    stamp.write_text(json.dumps(identity(), sort_keys=True) + "\n")
