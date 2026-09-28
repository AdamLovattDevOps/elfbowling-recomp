// Scene methods, 0x4099dc-0x409c50.
#include <elf/funcs.h>

extern "C" void fn_4099dc(TScene *sc)
{
    for (int i = 0; i < sc->nsprites; i++) {
        TGraphicSprite *s = sc->sprites[i];
        if (s->type == 3)
            fn_408f84((TButtonSprite *)s);
        else
            fn_40610c(s);
    }
}

extern "C" void fn_409a28(TScene *sc)
{
    for (int i = 0; i < sc->nsprites; i++) {
        TGraphicSprite *s = sc->sprites[i];
        if (s->f1d0)
            fn_408924(s);
    }
}

extern "C" void fn_409a6c(TScene *sc)
{
    sc->active = 1;
    TStage *e = sc->stage;
    e->down.x = -1;
    e->down.y = -1;
    TStage *e2 = sc->stage;
    e2->up.x = -1;
    e2->up.y = -1;
    sc->startTime = fn_4024bc(0);
    sc->ticks = 0;
    if (sc->onStart) {
        sc->onStart(sc, sc->started);
        sc->started = 1;
    }
    fn_40e4d8(g_455528, sc);
    fn_4099dc(sc);
    if (sc->onStart2)
        sc->onStart2(sc);
    fn_402318("Start of Scene ", sc->name);
}

extern "C" void fn_409b44(TScene *sc)
{
    fn_40ccd0(sc->stage->sound, sc);
    fn_40cbd0(sc->stage->sound, sc);
    sc->active = 0;
    sc->stopTime = fn_4024bc(0);
    fn_409c18(sc);
    fn_409a28(sc);
    fn_40cf24(sc->stage->sound);
}

extern "C" void fn_409bb8(TScene *sc, TStage *e)
{
    sc->stage = e;
}

extern "C" void fn_409bcc(TScene *sc, TGraphicSprite *s)
{
    if (sc->nsprites < 255) {
        s->index = sc->nsprites;
        sc->sprites[sc->nsprites++] = s;
    }
}

extern "C" void fn_409c00(TScene *sc, TSceneButton *b)
{
    fn_434b6c(b->ctl);
    b->visible = 0;
}

extern "C" void fn_409c18(TScene *sc)
{
    for (int i = 0; i < sc->nbuttons; i++)
        fn_409c00(sc, &sc->buttons[i]);
}
