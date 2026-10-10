/* SPDX-License-Identifier: GPL-2.0-or-later */
#define __EMSCRIPTEN__ 1
#include "../../patches/retrom_seekable_file.h"
#include <cassert>
#include <cstring>
#include <vector>
static std::vector<unsigned char> source(600000);
static bool failRead = false;
#ifdef RETROM_TEST_ASYNC_RANGE
#include <emscripten.h>
EM_JS(int, retrom_test_read, (const void *source, unsigned offset, unsigned length, void *out, int fail), {
    return Asyncify.handleSleep(wake => {
        Promise.resolve().then(() => {
            if (fail) { wake(-1); return; }
            HEAPU8.set(HEAPU8.slice(source + offset, source + offset + length), out);
            wake(length);
        });
    });
});
#endif
extern "C" double retrom_flycast_range_size(const char *path) { return std::strcmp(path, "disc.cdi") ? -1 : source.size(); }
extern "C" int retrom_flycast_range_read(double offset, unsigned length, void *out) {
    assert(length <= 256 * 1024 && offset >= 0 && offset + length <= source.size());
#ifdef RETROM_TEST_ASYNC_RANGE
    return retrom_test_read(source.data(), offset, length, out, failRead);
#else
    if (failRead) return -1;
    std::memcpy(out, source.data() + static_cast<size_t>(offset), length);
    return length;
#endif
}
int main() {
    for (size_t i = 0; i < source.size(); i++) source[i] = i % 251;
    FILE *metadata = retrom_open_seekable_file("disc.cdi"), *track = retrom_open_seekable_file("disc.cdi");
    assert(metadata && track && !retrom_open_seekable_file("bios.bin"));
    unsigned char footer[8];
    assert(!fseek(metadata, -8, SEEK_END) && fread(footer, 1, 8, metadata) == 8);
    assert(!std::memcmp(footer, source.data() + source.size() - 8, 8));
    assert(ftell(metadata) == static_cast<long>(source.size()) && ftell(track) == 0);
    std::vector<unsigned char> bytes(source.size());
    assert(fread(bytes.data(), 1, bytes.size(), track) == bytes.size() && bytes == source);
    assert(fread(footer, 1, 1, metadata) == 0 && feof(metadata));
    assert(fseek(metadata, -static_cast<long>(source.size()) - 1, SEEK_END) != 0);
    assert(fseek(metadata, 1, SEEK_END) != 0);
    assert(!fseek(track, 16, SEEK_SET) && !fseek(track, -8, SEEK_CUR) && ftell(track) == 8);
    fclose(metadata); // Closing metadata must not close the other track or the shared reader.
    assert(fread(footer, 1, 8, track) == 8 && !std::memcmp(footer, source.data() + 8, 8));
    fclose(track);
    track = retrom_open_seekable_file("disc.cdi");
    failRead = true;
    assert(fread(footer, 1, 8, track) == 0 && ferror(track));
    fclose(track);
}
