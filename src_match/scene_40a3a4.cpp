// TScene: create a text-box sprite fitted inside another sprite's frame, 0x40a3a4.
#include <elf/funcs.h>

extern "C" TGraphicSprite *fn_40a3a4(TScene *sc, const char *name, TGraphicSprite *ref, char f, int ml, int mr, int mt, int mb)
{
    ERect *p = (ERect *)&ref->casts[ref->frame]->bounds;
    ERect r;
    r = *p;
    r.l += ml;
    r.r -= mr;
    r.t += mt;
    r.b -= mb;
    fn_406dd0(ref, &r);
    int w = r.r - r.l;
    int h = r.b - r.t;
    TPoint pt = fn_43a8c4(ref->x / 1000, ref->y / 1000);
    pt.x = (r.l + r.r) / 2;
    pt.y = (r.t + r.b) / 2;
    TTextCast *tb = new TTextCast(sc->stage, name, w, h, f);
    TGraphicSprite *s = new TGraphicSprite(name, sc->stage);
    fn_4066c4(s, tb, pt);
    fn_409bcc(sc, s);
    return s;
}
