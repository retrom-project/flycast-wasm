/* SPDX-License-Identifier: GPL-2.0-or-later */
#include <stddef.h>
#include <string.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>

EM_JS(double, retrom_flycast_range_size_js, (const char *path), {
    const bridge = globalThis.RETROM_FLYCAST_RANGE;
    return bridge && UTF8ToString(path).split('/').pop() === bridge.filename ? bridge.sizeBytes : -1;
});

EM_JS(int, retrom_flycast_range_read_js, (double offset, unsigned length, void *destination), {
    return Asyncify.handleSleep(function(wakeUp) {
        const bridge = globalThis.RETROM_FLYCAST_RANGE;
        if (!bridge || !length || length > 256 * 1024) { wakeUp(-1); return; }
        let started = false;
        try {
            bridge.begin();
            started = true;
            Promise.resolve(bridge.read(offset, length)).then(function(bytes) {
                try {
                    if (!bytes || bytes.length !== length) { wakeUp(-1); return; }
                    HEAPU8.set(bytes, destination);
                    wakeUp(bytes.length);
                } finally { bridge.end(); }
            }, function(error) {
                try { wakeUp(-1); } finally { bridge.end(); bridge.fail(error); }
            });
        } catch (error) {
            if (started) bridge.end();
            wakeUp(-1);
            bridge.fail(error);
        }
    });
});

double retrom_flycast_range_size(const char *path) {
    return retrom_flycast_range_size_js(path);
}

int retrom_flycast_range_read(double offset, unsigned length, void *destination) {
    return retrom_flycast_range_read_js(offset, length, destination);
}
#endif

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
