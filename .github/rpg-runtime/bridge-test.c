#include <assert.h>
#include <string.h>
#include "bridge.c"
const unsigned char *__real_glGetString(unsigned int name) { (void)name; return (const unsigned char *)"vendor"; }
int main(void) {
    assert(strcmp((const char *)__wrap_glGetString(0x1f02), "OpenGL ES 3.0 WebGL 2.0") == 0);
    assert(strcmp((const char *)__wrap_glGetString(0x8b8c), "OpenGL ES GLSL ES 3.00") == 0);
    assert(strcmp((const char *)__wrap_glGetString(0x1f00), "vendor") == 0);
    char out[32];
    fill_short_pathname_representation_noext(out, "dir/game.chd", sizeof out);
    assert(strcmp(out, "game") == 0);
    fill_short_pathname_representation(out, "C:\\dir\\game.chd", sizeof out);
    assert(strcmp(out, "game.chd") == 0);
    fill_short_pathname_representation(out, "game.chd", 1);
    assert(out[0] == 0);
    fill_short_pathname_representation(NULL, "game.chd", 0);
    return 0;
}
