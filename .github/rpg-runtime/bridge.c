/* SPDX-License-Identifier: GPL-2.0-or-later */
#include <stddef.h>
#include <string.h>

extern const unsigned char *__real_glGetString(unsigned int name);
const unsigned char *__wrap_glGetString(unsigned int name)
{
    if (name == 0x1f02) return (const unsigned char *)"OpenGL ES 3.0 WebGL 2.0";
    if (name == 0x8b8c) return (const unsigned char *)"OpenGL ES GLSL ES 3.00";
    return __real_glGetString(name);
}

/* WASM backend owns block invalidation; native block manager is unused. */
void bm_Reset(void) {}

void fill_short_pathname_representation(char *out, const char *path, size_t size)
{
    const char *base = strrchr(path, '/');
    const char *back = strrchr(path, '\\');
    if (back && (!base || back > base)) base = back;
    base = base ? base + 1 : path;
    if (size) {
        size_t length = strlen(base);
        if (length >= size) length = size - 1;
        memcpy(out, base, length);
        out[length] = 0;
    }
}

void fill_short_pathname_representation_noext(char *out, const char *path, size_t size)
{
    fill_short_pathname_representation(out, path, size);
    if (size) {
        char *dot = strrchr(out, '.');
        if (dot) *dot = 0;
    }
}
