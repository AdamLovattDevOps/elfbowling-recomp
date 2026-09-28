// TScene sprite factories, 0x409fc8-0x40a3a4.
#include <elf/funcs.h>

extern "C" TGraphicSprite *fn_409fc8(TScene *sc, const char *name, const char *cast, TPoint p)
{
    TGraphicSprite *s = new TGraphicSprite(name, sc->stage);
    fn_406780(s, cast, p);
    fn_409bcc(sc, s);
    return s;
}

extern "C" TGraphicSprite *fn_40a06c(TScene *sc, const char *name, const char *cast)
{
    TStage *e = sc->stage;
    return fn_409fc8(sc, name, cast, fn_43a8c4((e->screen.right - e->screen.left) / 2, (e->screen.bottom - e->screen.top) / 2));
}

extern "C" TButtonSprite *fn_40a288(TScene *sc, const char *name, const char *b, const char *c, TPoint pt)
{
    TButtonSprite *p = fn_40a1fc(sc, name, b, c, pt);
    fn_409bcc(sc, p);
    return p;
}

extern "C" TButtonSprite *fn_40a2c0(TScene *sc, const char *name, const char *b, const char *c)
{
    TStage *e = sc->stage;
    return fn_40a288(sc, name, b, c, fn_43a8c4((e->screen.right - e->screen.left) / 2, (e->screen.bottom - e->screen.top) / 2));
}

extern "C" void fn_40a348(TScene *sc, const char *n1, const char *n2)
{
    TGraphicSprite *a = fn_40a598(sc, n1);
    TGraphicSprite *b = fn_40a598(sc, n2);
    TPoint p = a->f0f0;
    if (p.x != 999999999)
        fn_408740(b, p.x, p.y);
}
