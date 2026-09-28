// TScene::fn_40a1fc: new TButtonSprite (0x3cc bytes) on this scene's stage.
// Member function: as a free function sc->stage loads into edx instead of ecx.
#include <elf/funcs.h>

TButtonSprite *TScene::fn_40a1fc(const char *name, const char *b, const char *c, TPoint p)
{
    TButtonSprite *r = new TButtonSprite(stage, name, b, c, p);
    return r;
}
// MATCH 40a1fc @TScene@fn_40a1fc$qpxct1t18tagPOINT
