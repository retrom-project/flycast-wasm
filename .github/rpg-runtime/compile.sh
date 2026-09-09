#!/usr/bin/env bash
set -euo pipefail
cd /work
# Fixed flyinghead/flycast input commit time; never embed the build wall clock.
export SOURCE_DATE_EPOCH=1771589293
export SOURCE_DIR=/work/.cache/flycast
export BUILD_DIR=/work/.cache/flycast/build-wasm-prod
export EJS_RA=/work/.cache/retroarch
export STUBS=/work/.cache/bridge.o
emcc -O3 -flto -c .github/rpg-runtime/bridge.c -o "$STUBS"
emmake make -C "$EJS_RA" -f Makefile.emulatorjs -f /work/.github/rpg-runtime/objects.mk   retrom-objects HAVE_CHD=0 HAVE_THREADS=0 PTHREAD_POOL_SIZE=0 ASYNC=1 HAVE_OPENGLES3=1   HAVE_AL=1 HAVE_RWEBAUDIO=0 GIT_VERSION=6dd4353937ef48b6ec0bfbdbb15d1c5992d86927 -j8
emcmake cmake -S "$SOURCE_DIR" -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE=Release   -DLIBRETRO=ON -DUSE_GLES=ON -DUSE_LUA=OFF   -DCMAKE_C_FLAGS="-DJIT_PROD_BUILD=1 -DFLY_RELEASE_BUILD=1"   -DCMAKE_CXX_FLAGS="-DJIT_PROD_BUILD=1 -DFLY_RELEASE_BUILD=1"
emmake make -C "$BUILD_DIR" -j8
bash .github/rpg-runtime/link.sh
