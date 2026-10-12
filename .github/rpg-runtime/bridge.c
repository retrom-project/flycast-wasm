/* SPDX-License-Identifier: GPL-2.0-or-later */
#include <stddef.h>
#include <string.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>

EM_JS(int, retrom_flycast_range_open_js, (const char *path), {
    const bridge = globalThis.RETROM_FLYCAST_RANGE;
    return bridge ? bridge.open(UTF8ToString(path)) : 0;
});
EM_JS(double, retrom_flycast_range_size_js, (int handle), {
    const bridge = globalThis.RETROM_FLYCAST_RANGE;
    return bridge ? bridge.size(handle) : -1;
});
EM_JS(void, retrom_flycast_range_close_js, (int handle), {
    const bridge = globalThis.RETROM_FLYCAST_RANGE;
    if (bridge) bridge.close(handle);
});

EM_JS(int, retrom_flycast_range_read_js, (int handle, double offset, unsigned length, void *destination), {
    return Asyncify.handleSleep(function(wakeUp) {
        const bridge = globalThis.RETROM_FLYCAST_RANGE;
        if (!bridge || !length || length > 256 * 1024) { wakeUp(-1); return; }
        let started = false;
        try {
            bridge.begin();
            started = true;
            const complete = function(bytes) {
                try {
                    if (!bytes || bytes.length !== length) { wakeUp(-1); return; }
                    HEAPU8.set(bytes, destination);
                    wakeUp(bytes.length);
                } finally { bridge.end(); }
            };
            const bytes = bridge.read(handle, offset, length);
            // Cache hits must remain synchronous across dynamically compiled SH4 code.
            if (bytes && typeof bytes.then !== 'function') complete(bytes);
            else Promise.resolve(bytes).then(complete, function(error) {
                try { wakeUp(-1); } finally { bridge.end(); bridge.fail(error); }
            });
        } catch (error) {
            if (started) bridge.end();
            wakeUp(-1);
            bridge.fail(error);
        }
    });
});

int retrom_flycast_range_open(const char *path) { return retrom_flycast_range_open_js(path); }
double retrom_flycast_range_size(int handle) { return retrom_flycast_range_size_js(handle); }
void retrom_flycast_range_close(int handle) { retrom_flycast_range_close_js(handle); }
int retrom_flycast_range_read(int handle, double offset, unsigned length, void *destination) {
    return retrom_flycast_range_read_js(handle, offset, length, destination);
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
