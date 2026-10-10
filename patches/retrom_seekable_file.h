/* SPDX-License-Identifier: GPL-2.0-or-later */
#pragma once
#include <cstdio>
#include <cstdint>
#include <cerrno>
#include <algorithm>
#include "retrom_flycast_range.h"

// A separate cursor per FILE supports CDI metadata and concurrent track handles.
// The bytes remain owned by the bounded content-io reader; no disc materialization.
struct RetromSeekableFile {
    int64_t size;
    int64_t position = 0;
};
inline ssize_t retrom_stdio_read(void *cookie, char *bytes, size_t length) {
    auto *file = static_cast<RetromSeekableFile *>(cookie);
    const size_t count = std::min<uint64_t>({length, static_cast<uint64_t>(file->size - file->position), 256 * 1024});
    if (!count) return 0;
    const int received = retrom_flycast_range_read(file->position, count, bytes);
    if (received != static_cast<int>(count)) { errno = EIO; return -1; }
    file->position += received;
    return received;
}
inline int retrom_stdio_seek(void *cookie, off_t *offset, int origin) {
    auto *file = static_cast<RetromSeekableFile *>(cookie);
    const int64_t base = origin == SEEK_SET ? 0 : origin == SEEK_CUR ? file->position : origin == SEEK_END ? file->size : -1;
    if (base < 0 || *offset < -base || *offset > file->size - base) { errno = EINVAL; return -1; }
    file->position = base + *offset;
    *offset = file->position;
    return 0;
}
inline int retrom_stdio_close(void *cookie) { delete static_cast<RetromSeekableFile *>(cookie); return 0; }
inline FILE *retrom_open_seekable_file(const char *path) {
    const double size = retrom_flycast_range_size(path);
    if (size <= 0 || size > INT32_MAX) { errno = EINVAL; return nullptr; }
    auto *file = new RetromSeekableFile{static_cast<int64_t>(size)};
    FILE *stream = fopencookie(file, "rb", {retrom_stdio_read, nullptr, retrom_stdio_seek, retrom_stdio_close});
    if (!stream) delete file;
    return stream;
}
