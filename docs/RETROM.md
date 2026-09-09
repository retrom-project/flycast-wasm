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

Prerequisites: Git, Docker, Python 3, 7-Zip, and a C compiler for the bridge test.
Run from a non-root PFB checkout:

```bash
mkdir -p .cache
cc -Wall -Wextra -Werror .github/rpg-runtime/bridge-test.c -o .cache/bridge-test
.cache/bridge-test
# From the Retrom checkout in the same PFB:
make pfb-core-build PFB=<name> CORE=flycast
```

The underlying interface is `.github/rpg-runtime/build-candidate.sh <absolute-empty-output-directory>`.
It produces `flycast-wasm.data`, `flycast.json`, `LICENSE`, and a closed,
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

The current Retrom target is single-file Dreamcast CHD, WebGL2, no pthreads.
The frontend uses `/` as its content/system directory, so the host supplies
`/dc/dc_boot.bin` and `/dc/dc_flash.bin` as external files. Flash contains mutable
console settings. Windows CE/MMU games, NAOMI, Atomiswave, multi-disc switching,
and multiplayer netplay are outside this initial target's supported scope.

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
and publishes the three payload files plus `rpg-runtime-release.json`.
The release metadata records repository, tag, commit, ABI and exact file hashes.
Run `python3 -B -m unittest discover -s .github/rpg-runtime -p 'test_*.py'`
to verify that dirty sources, wrong identities and altered bytes are rejected.
