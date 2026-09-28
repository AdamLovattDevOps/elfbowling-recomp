// TScene: clone a sprite's casts into a new "#"-prefixed sprite, 0x40a0d8.
// Note: arrays are placed below scalar locals even when declared first.
#include <string.h>
#include <elf/funcs.h>

extern "C" TGraphicSprite *fn_40a0d8(TScene *sc, const char *name, const char *srcName)
{
    char buf[0x1c];
    buf[0] = '#';
    TGraphicSprite *src = fn_40a598(sc, srcName);
    strcpy(buf + 1, src->casts[0]->name);
    TGraphicSprite *dst;
    TStage *e = sc->stage;
    dst = fn_409fc8(sc, name, buf, fn_43a8c4((e->screen.right - e->screen.left) / 2, (e->screen.bottom - e->screen.top) / 2));
    for (int i = 1; i < src->ncasts; i++) {
        strcpy(buf + 1, src->casts[i]->name);
        fn_406724(dst, buf);
        if (src->talkCasts[i]) {
            strcpy(buf + 1, src->talkCasts[i]->name);
            fn_4067e0(dst, buf, i);
        }
    }
    return dst;
}
