// TScene members 0x40a520/0x40a598, matched as real C++ members (this at
// [ebp+8], cdecl). As free functions the compiler orders the loads for
// this->arr[i] / i < this->n differently.
#include <elf/funcs.h>

int TScene::fn_40a520(const char *nm)
{
    int idx = -1;
    for (int i = 0; idx == -1 && i < nsprites; i++) {
        TGraphicSprite *s = sprites[i];
        if (stricmp(s->name, nm) == 0)
            idx = i;
    }
    if (idx == -1)
        fn_401f70(name, " Scene is unable to locate sprite ", nm);
    return idx;
}

TGraphicSprite *TScene::fn_40a598(const char *nm)
{
    TGraphicSprite *r = 0;
    int i = fn_40a520(nm);
    if (i >= 0)
        r = sprites[i];
    return r;
}
// MATCH 40a520 @TScene@fn_40a520$qpxc
// MATCH 40a598 @TScene@fn_40a598$qpxc
