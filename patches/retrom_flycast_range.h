#pragma once

#ifdef __EMSCRIPTEN__
#ifdef __cplusplus
extern "C" {
#endif
double retrom_flycast_range_size(const char *path);
int retrom_flycast_range_read(double offset, unsigned length, void *destination);
#ifdef __cplusplus
}
#endif
#endif
