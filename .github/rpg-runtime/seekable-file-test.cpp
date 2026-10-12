/* SPDX-License-Identifier: GPL-2.0-or-later */
#define __EMSCRIPTEN__ 1
#include "../../patches/retrom_seekable_file.h"
#include <cassert>
#include <cstring>
#include <vector>
#include <map>
static std::vector<unsigned char> source(600000);
static bool failRead = false;
static std::vector<unsigned char> second(8192, 37);
static std::map<int, bool> handles;
static int nextHandle = 0;
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
extern "C" int retrom_flycast_range_open(const char *path) {
    bool other = !std::strcmp(path, "other/disc.cdi");
    if (!other && std::strcmp(path, "disc.cdi")) return 0;
    handles[++nextHandle] = other;
    return nextHandle;
}
extern "C" double retrom_flycast_range_size(int handle) {
    return handles.count(handle) ? (handles.at(handle) ? second.size() : source.size()) : -1;
}
extern "C" void retrom_flycast_range_close(int handle) { assert(handles.erase(handle) == 1); }
extern "C" int retrom_flycast_range_read(int handle, double offset, unsigned length, void *out) {
    const auto &bytes = handles.at(handle) ? second : source;
    assert(length <= 256 * 1024 && offset >= 0 && offset + length <= bytes.size());
#ifdef RETROM_TEST_ASYNC_RANGE
    return retrom_test_read(bytes.data(), offset, length, out, failRead);
#else
    if (failRead) return -1;
    std::memcpy(out, bytes.data() + static_cast<size_t>(offset), length);
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
    FILE *other = retrom_open_seekable_file("other/disc.cdi");
    assert(other && fread(footer, 1, 8, other) == 8 && footer[0] == 37);
    fclose(other);
    fclose(metadata); // Closing metadata must not close the other track or the shared reader.
    assert(fread(footer, 1, 8, track) == 8 && !std::memcmp(footer, source.data() + 8, 8));
    fclose(track);
    track = retrom_open_seekable_file("disc.cdi");
    failRead = true;
    assert(fread(footer, 1, 8, track) == 0 && ferror(track));
    fclose(track);
    assert(handles.empty());
}
