// TButtonSprite methods, 0x408f84-0x4092bc (old: Actor): reset, blink setup
// and the hover/click/blink sounds.
#include <elf/funcs.h>

extern "C" void fn_408f84(TButtonSprite *s)
{
    TButtonSprite *b = s;
    fn_40610c(b);
    s->blink = 0;
    s->onTime = 0;
    s->offTime = 0;
    s->blinkCount = 0;
    s->blinkN = 0;
    s->lit = 0;
    s->nextTime = 0;
    s->hover = 0;
    s->blinkSnd = -1;
}

extern "C" void fn_409000(TButtonSprite *s, int a, int b, int c, int d)
{
    s->blink = 1;
    s->onTime = c;
    s->offTime = d;
    s->blinkCount = a;
    s->blinkN = 0;
    s->lit = 0;
    s->blinkSnd = -1;
    s->nextTime = fn_4024bc(b);
}

extern "C" void fn_409068(TButtonSprite *s, const char *id, int arg)
{
    if (id == 0) {
        s->hoverSnd = -2;
        return;
    }
    s->hoverSnd = fn_40cdec(s->stage->sound, id);
    s->hoverSndArg = arg;
}

extern "C" void fn_4090b4(TButtonSprite *s, const char *id, int arg)
{
    if (id == 0) {
        s->blinkSnd = -2;
        return;
    }
    s->blinkSnd = fn_40cdec(s->stage->sound, id);
    s->blinkSndArg = arg;
}

extern "C" void fn_4091ac(TButtonSprite *s)
{
    TSoundMgr *g = s->stage->sound;
    if (s->hoverSnd >= 0) {
        fn_40cae0(g, 0, s, s->hoverSnd, s->hoverSndArg, 0);
        return;
    }
    if (s->hoverSnd != -2 && g->defA >= 0)
        fn_40cae0(g, 0, s, g->defA, g->defAarg, 0);
}

extern "C" void fn_409234(TButtonSprite *s)
{
    TSoundMgr *g = s->stage->sound;
    if (s->clickSnd >= 0) {
        fn_40cae0(g, 0, s, s->clickSnd, s->clickSndArg, 0);
        return;
    }
    if (s->hoverSnd != -2 && g->defB >= 0)
        fn_40cae0(g, 0, s, g->defB, g->defBarg, 0);
}
