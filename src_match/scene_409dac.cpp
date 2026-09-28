// TScene tick: run due sprite updates, 0x409dac.
#include <elf/funcs.h>

extern "C" int fn_409dac(TScene *sc)
{
    int now = fn_4024bc(0);
    int elapsed = (now - sc->startTime) / sc->period;
    int n = tmin(elapsed - sc->ticks, 3);
    for (int k = 0; k < n; k++) {
        char a = 1;
        char b = 1;
        char c = 1;
        for (int i = sc->nsprites - 1; i >= 0; i--) {
            TGraphicSprite *s = sc->sprites[i];
            fn_408760(s, now);
            if (s->shown) {
                if (s->type == 3) {
                    TButtonSprite *t = (TButtonSprite *)s;
                    fn_4092bc(t, sc->stage, now, &a, &b, &c);
                } else if (s->clickable || s->draggable) {
                    fn_408940(s, sc->stage, now, &a, &b, &c);
                }
                fn_4087c8(s, sc, now);
            }
        }
        if (a && sc->stage->down.x >= 0 && sc->onDown) {
            sc->onDown(sc);
            TStage *e = sc->stage;
            e->down.x = -1;
            e->down.y = -1;
        }
        if (b && sc->stage->up.x >= 0 && sc->onUp) {
            sc->onUp(sc);
            TStage *e2 = sc->stage;
            e2->up.x = -1;
            e2->up.y = -1;
        }
        now = fn_4024bc(0);
        sc->ticks++;
    }
    return 0;
}
