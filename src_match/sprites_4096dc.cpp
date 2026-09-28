// Sprite group methods, 0x4096dc-0x40971c.
// fn_40971c..fn_4097e8 are member functions: see sprites_40971c.cpp.
#include <elf/funcs.h>

extern "C" char fn_4096dc(TSpriteGroup *g, TGraphicSprite *s)
{
    char r = 0;
    if (g->count < 64) {
        g->items[g->count++] = s;
        r = 1;
    } else {
        fn_401f30("Too many sprites in group ", g->name);
    }
    return r;
}
