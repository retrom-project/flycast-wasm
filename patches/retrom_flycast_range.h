#pragma once

#ifdef __EMSCRIPTEN__
#ifdef __cplusplus
extern "C" {
#endif
int retrom_flycast_range_open(const char *path);
double retrom_flycast_range_size(int handle);
int retrom_flycast_range_read(int handle, double offset, unsigned length, void *destination);
void retrom_flycast_range_close(int handle);
#ifdef __cplusplus
}
#endif
#endif
