#!/usr/bin/env bash
set -euo pipefail
root=$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)
output=${1:?output required}
cd "$root"
python3 .github/rpg-runtime/prepare-source.py
docker run --rm --user "$(id -u):$(id -g)"   -e EM_CACHE=/work/.cache/emscripten -v "$root:/work" -w /work   emscripten/emsdk@sha256:90b757eb11fa9a0e3ce4d2d9f76d932a56018e4accc37b5a28b2783751e60eb7   bash .github/rpg-runtime/compile.sh
python3 .github/rpg-runtime/check-link.py .cache/flycast/build-wasm-prod/flycast_libretro.js
cc -Wall -Wextra -Werror .github/rpg-runtime/bridge-test.c -o .cache/bridge-test
.cache/bridge-test
python3 .github/rpg-runtime/package.py "$output"
