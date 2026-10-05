# Retrom Flycast candidate

Retrom consumes this fork through the EmulatorJS Provider. The maintained upstream
baseline is nasomers/flycast-wasm v1.0, commit
`16e9c7b4b7065a4bd42e7490e8aab3e9283d5415`. `main` remains an upstream mirror;
`retrom/1.0` is the maintenance branch. Feature branches use `feat/*`, `fix/*`,
`build/*`, or `sync/upstream-*`.

The build materializes exact inputs in the ignored `.cache/` directory:

| Input | Revision |
| --- | --- |
| flyinghead/flycast | `2c48c0188a2afc158b02b6d1865d898756a03071` and its recursive submodules |
| EmulatorJS/RetroArch | `6dd4353937ef48b6ec0bfbdbb15d1c5992d86927` |
| emscripten/emsdk | `sha256:90b757eb11fa9a0e3ce4d2d9f76d932a56018e4accc37b5a28b2783751e60eb7` |

Prerequisites: Git, Docker, Python 3, 7-Zip, a C compiler for the bridge test, and g++ for the ROM table exporter.
Run from a non-root PFB checkout:

```bash
mkdir -p .cache
cc -Wall -Wextra -Werror .github/rpg-runtime/bridge-test.c -o .cache/bridge-test
.cache/bridge-test
# From the Retrom checkout in the same PFB:
make pfb-core-build PFB=<name> CORE=flycast
```

The underlying interface is `.github/rpg-runtime/build-candidate.sh <absolute-empty-output-directory>`.
It produces `flycast-wasm.data`, `flycast.json`, `flycast-rom-requirements.json`, `LICENSE`, and a closed,
SHA-256-verified `retrom-core-candidate.json`. Generated inputs are disposable;
do not edit `.cache/flycast` or `.cache/retroarch` manually. Change the fork's
tracked patches/build scripts instead. The package does not contain a BIOS or game.

The bridge replaces the original precompiled stub with source, preserving the
WebGL ES version queries and file path helpers needed at link time. The link
exports `HEAPU8`, `HEAPU32`, and `HEAP16`: the upstream JIT and audio callback
access these on the Emscripten Module, including after memory growth.

Compilation fixes `SOURCE_DATE_EPOCH` to `1771589293`, the pinned Flycast input
commit's timestamp, so RetroArch's embedded build date is deterministic. A cache
created before this setting must be cleaned before a reproducibility comparison.
RetroArch's embedded `GIT_VERSION` is the complete pinned 40-character commit;
it must not depend on Git's repository-size-dependent automatic abbreviation.

Retrom uses this core for single-file Dreamcast CHD and NAOMI, NAOMI2, and
Atomiswave ZIP cartridges, with WebGL2 and no pthreads. The native CHD and ZIP
readers use the same asynchronous, bounded Range bridge supplied by the runtime;
neither format requires a complete game download before native startup. ZIP
cartridges may still read most archive members while the arcade machine boots.
The frontend uses `/` as its content/system directory. Dreamcast needs
`/dc/dc_boot.bin` and `/dc/dc_flash.bin`; the arcade targets need their matching
BIOS ZIP in `/dc/`. Flash contains mutable console settings. Windows CE/MMU
games, multi-disc switching, and multiplayer netplay remain outside the current
supported scope.

Candidates are for PFB validation. Do not create a stable core release until
Retrom's real review preview, product launch, standard gamepad input, bounded
instant checkpoint, a new Launch restore, and input after restore all pass.
Release tags must follow `retrom-core-1.0-rN` (optional `-rc.N`); published tags
and assets are immutable. A formal release additionally needs the release
metadata described by `retrom-fork.json`, corresponding source, and a clean
reproducibility run. No formal release is created by the candidate wrapper.

`retrom-quality.yml` builds and audits PRs into `retrom/1.0`. After merging a
validated PR, create an annotated `retrom-core-1.0-rN` tag on its maintenance
commit. `retrom-release.yml` verifies the tag and ancestry, rebuilds from fixed
inputs, validates the clean candidate's identity and bytes with `release.py`,
and publishes the four payload files plus `rpg-runtime-release.json`.
The release metadata records repository, tag, commit, ABI and exact file hashes.
Run `python3 -B -m unittest discover -s .github/rpg-runtime -p 'test_*.py'`
to verify that dirty sources, wrong identities and altered bytes are rejected.

The ROM requirements artifact is compiled from the same prepared `Games` table
as the core. It identifies cartridge versus GD-ROM media, hardware family,
parent name and each file's size/CRC and optional status. Internal Copy blobs
are not files; Eeprom defaults are optional, while EepromBE16 remains required.
Duplicate machine names follow the core's first-match lookup. It binds the
packaged core SHA-256, pinned source commit, table digest and exporter digest.
Hosts can reject unsupported GD-ROM/PIC-only and incomplete split sets without
reading C++ source or searching global game directories. Publishing this catalog
does not add GD-ROM or external Parent delivery support.
